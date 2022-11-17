#include "mapper.hpp"

#include "../../util/xorstr.h"

namespace util
{
	inline uint16_t reloc_extract_type(const uint16_t relocation) { return (relocation >> 12) & 0x0ff; }
	inline uint16_t reloc_extract_rva(const uint16_t relocation)  { return (relocation >> 00) & 0xfff; }

	inline PIMAGE_NT_HEADERS get_nt_headers(const uint8_t* buffer)
	{
		const auto dos_header = PIMAGE_DOS_HEADER(buffer);
		if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
			return nullptr;

		const auto nt_headers = PIMAGE_NT_HEADERS(buffer + dos_header->e_lfanew);
		if (nt_headers->Signature != IMAGE_NT_SIGNATURE || 
			nt_headers->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)
			return nullptr;

		constexpr auto current_machine = sizeof(void*) == 8 ? 
			IMAGE_FILE_MACHINE_AMD64 : IMAGE_FILE_MACHINE_I386;
		if (nt_headers->FileHeader.Machine != current_machine)
			return nullptr;

		return nt_headers;
	}
}

Mapper::Mapper(const uint8_t* file_buffer)
{
	const auto file_nt_headers = util::get_nt_headers(file_buffer);
	if (!file_nt_headers)
		throw InvalidHeaderException();

	image = std::make_unique<uint8_t[]>(file_nt_headers->OptionalHeader.SizeOfImage);

	const auto section_header = IMAGE_FIRST_SECTION(file_nt_headers);
	for (size_t i = 0; i < file_nt_headers->FileHeader.NumberOfSections; ++i)
	{
		const auto& section = section_header[i];
		std::memcpy(section.VirtualAddress   + image.get(),
					section.PointerToRawData + file_buffer,
					section.SizeOfRawData);
	}

	std::memcpy(image.get(), file_buffer, file_nt_headers->OptionalHeader.SizeOfHeaders);
	//std::printf(xorstr_("Copied PE image sections.\n"));

	nt_headers = util::get_nt_headers(image.get());
	if (!nt_headers)
		throw InvalidHeaderException();
}

bool Mapper::relocate_image(const uintptr_t new_base) const noexcept
{
	auto [relocation, relocation_size] = get_data_directory<PIMAGE_BASE_RELOCATION>(IMAGE_DIRECTORY_ENTRY_BASERELOC);
	if (!relocation || !relocation_size)
	{
		//std::printf(xorstr_("Image has no relocations.\n"));
		return true;
	}

	const auto delta = new_base - nt_headers->OptionalHeader.ImageBase;
	const auto relocation_end = uintptr_t(relocation) + relocation_size;

	for (; uintptr_t(relocation) < relocation_end; relocation = PIMAGE_BASE_RELOCATION(uintptr_t(relocation) + relocation->SizeOfBlock))
	{
		const auto chains = (uint16_t*)(relocation + 1);
		for (size_t i = 0; i < (relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t); ++i)
		{
			const auto current = chains[i];
			const auto target  = from_rva<uintptr_t>(relocation->VirtualAddress) + util::reloc_extract_rva(current);

			switch (util::reloc_extract_type(current))
			{
			case IMAGE_REL_BASED_LOW:
				*(uint16_t*)target += LOWORD(uint32_t(delta));
				break;

			case IMAGE_REL_BASED_HIGH:
				*(uint16_t*)target += HIWORD(uint32_t(delta));
				break;

			case IMAGE_REL_BASED_HIGHLOW:
				*(uint32_t*)target += uint32_t(delta);
				break;

			case IMAGE_REL_BASED_DIR64:
				*(uint64_t*)target += delta;
				break;

			case IMAGE_REL_BASED_HIGHADJ:
			case IMAGE_REL_BASED_ABSOLUTE:
				break;

			default:
				//std::printf(xorstr_("Invalid relocation type: %X.\n"), util::reloc_extract_type(current));
				return false;
			}
		}
	}

	//std::printf(xorstr_("Relocated image with delta %X.\n"), delta);
	return true;
}

bool Mapper::resolve_imports(ModuleManager& module_manager) const noexcept
{
	auto [import_descriptor, descriptor_size] = get_data_directory<PIMAGE_IMPORT_DESCRIPTOR>(IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (!import_descriptor || !descriptor_size)
	{
		//std::printf(xorstr_("Image has no import table.\n"));
		return true;
	}

	for (; import_descriptor->Name; ++import_descriptor)
	{
		const auto module_name  = from_rva<const char*>(import_descriptor->Name);
		const auto module_entry = module_manager.find_module(module_name);
		if (!module_entry)
		{
			//std::printf(xorstr_("Module %s not found. Cannot fix IAT.\n"), module_name);
			return false;
		}

		auto func_entry   = from_rva<PIMAGE_THUNK_DATA>(import_descriptor->FirstThunk);
		auto lookup_entry = from_rva<PIMAGE_THUNK_DATA>(
			import_descriptor->OriginalFirstThunk ?
			import_descriptor->OriginalFirstThunk :
			import_descriptor->FirstThunk);

		for (; lookup_entry->u1.AddressOfData; ++lookup_entry, ++func_entry)
		{
			uintptr_t import_address;

			if (lookup_entry->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				import_address = uintptr_t(module_entry->get_export_by_ordinal(lookup_entry->u1.Ordinal & 0xffff));
			}
			else
			{
				const auto import_name = from_rva<PIMAGE_IMPORT_BY_NAME>(lookup_entry->u1.AddressOfData)->Name;
				import_address = uintptr_t(module_entry->get_export_by_name(import_name));
			}

			if (!import_address)
			{
				//std::printf(xorstr_("Module %s: import resolving failed.\n"), module_name);
				return false;
			}

			func_entry->u1.Function = import_address;
		}
	}

	//std::printf(xorstr_("Resolved image imports.\n"));
	return true;
}

// don't use replaced with strip_image( )
//void Mapper::erase_headers() const noexcept
//{
//	std::memset(image.get(), 0x00, nt_headers->OptionalHeader.SizeOfHeaders);
//}

void Mapper::strip_image( ) const noexcept
{
    const auto debug_directory = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
    if (debug_directory.VirtualAddress && debug_directory.Size)
    {
        for (size_t i = 0; i < debug_directory.Size / sizeof(IMAGE_DEBUG_DIRECTORY); ++i)
        {
            const auto directory = PIMAGE_DEBUG_DIRECTORY(image.get() +
                debug_directory.VirtualAddress + i * sizeof(IMAGE_DEBUG_DIRECTORY));

            if (directory->AddressOfRawData && directory->SizeOfData)
                std::memset(image.get() + directory->AddressOfRawData, 0x00, directory->SizeOfData);
        }
    }

    const auto directories = {
        IMAGE_DIRECTORY_ENTRY_EXPORT, IMAGE_DIRECTORY_ENTRY_RESOURCE, IMAGE_DIRECTORY_ENTRY_EXCEPTION,
        IMAGE_DIRECTORY_ENTRY_BASERELOC, IMAGE_DIRECTORY_ENTRY_DEBUG, IMAGE_DIRECTORY_ENTRY_ARCHITECTURE,
        IMAGE_DIRECTORY_ENTRY_GLOBALPTR, IMAGE_DIRECTORY_ENTRY_TLS, IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
        IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT, IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR
    };

    for (const auto directory_type : directories)
    {
        const auto [directory, size] = get_data_directory<uint8_t*>(directory_type);
        if (directory && size)
            std::memset(directory, 0x00, size);
    }

    const auto image_size = nt_headers->OptionalHeader.SizeOfImage;
    std::memset(image.get(), 0x00, nt_headers->OptionalHeader.SizeOfHeaders);

    //if (false)
    //{
    //	std::ofstream stream("mapped_binary.bin");
    //	stream.write((const char*)image.get(), image_size);
    //	stream.close();
    //}
}

std::pair<void*, size_t> Mapper::get_image() const noexcept
{
	return std::make_pair((void*)image.get(), nt_headers->OptionalHeader.SizeOfImage);
}

void* Mapper::get_entrypoint(const uintptr_t base_address) const noexcept
{
	const auto entrypoint_rva = nt_headers->OptionalHeader.AddressOfEntryPoint;
	if (!entrypoint_rva)
		return nullptr;

	return (void*)(base_address + entrypoint_rva);
}
