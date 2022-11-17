#include "module_mapper.hpp"
#include "util.hpp"

bool ModuleMapper::map_as_image(const std::vector<uint8_t>& file_buffer)
{
	const auto nt_headers = util::get_nt_headers(file_buffer.data());
	if (!nt_headers)
	{
		log_error("Input file has invalid PE headers.");
		return false;
	}

	image_buffer.resize(nt_headers->OptionalHeader.SizeOfImage);

	const auto section_header = IMAGE_FIRST_SECTION(nt_headers);
	for (size_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i)
	{
		const auto& section = section_header[i];
		std::memcpy(section.VirtualAddress + image_buffer.data(),
			section.PointerToRawData + file_buffer.data(),
			section.SizeOfRawData);
	}

	std::memcpy(image_buffer.data(), file_buffer.data(), nt_headers->OptionalHeader.SizeOfHeaders);

	this->nt_headers = util::get_nt_headers(image_buffer.data());
	this->local_base = image_buffer.data();

	if (!this->nt_headers)
	{
		log_error("Input file has invalid PE headers (2).");
		return false;
	}

	return true;
}

bool ModuleMapper::relocate(uintptr_t delta)
{
	auto [relocation, directory_size] = get_data_directory<PIMAGE_BASE_RELOCATION>(IMAGE_DIRECTORY_ENTRY_BASERELOC);
	if (!relocation || !directory_size)
	{
		if (relocation || directory_size)
		{
			log_error("Relocation directory is corrupted.");
			return false;
		}

		log_debug("Image has no relocations.");
		return true;
	}

	const auto relocation_end = uintptr_t(relocation) + directory_size;

	for (; uintptr_t(relocation) < relocation_end; relocation = PIMAGE_BASE_RELOCATION(uintptr_t(relocation) + relocation->SizeOfBlock))
	{
		const auto entries = (uint16_t*)(relocation + 1);
		for (size_t i = 0; i < (relocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t); ++i)
		{
			const auto entry  = entries[i];
			const auto target = from_rva<uintptr_t>(relocation->VirtualAddress) + (entry & 0xFFF);
			const auto type   = entry >> 12;

			switch (type)
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

			case IMAGE_REL_BASED_ABSOLUTE:
				break;

			default:
				log_error("Unsupported relocation type: %d.", type);
				return false;
			}
		}
	}

	return true;
}

bool ModuleMapper::resolve_imports()
{
	auto [import_descriptor, directory_size] = get_data_directory<PIMAGE_IMPORT_DESCRIPTOR>(IMAGE_DIRECTORY_ENTRY_IMPORT);
	if (!import_descriptor || !directory_size)
	{
		if (import_descriptor || directory_size)
		{
			log_error("Import directory is corrupted.");
			return false;
		}

		log_debug("Image has no imports.");
		return true;
	}

	for (; import_descriptor->Name; ++import_descriptor)
	{
		const auto name = from_rva<const char*>(import_descriptor->Name);
		if (!name)
		{
			log_error("No name in import descriptor.\n");
			return false;
		}

		const auto module_name = std::string(name);
		const auto wmodule_name = std::wstring(module_name.begin(), module_name.end());

		std::wstring resolved_name;
		auto module_entry = mapper.get_process_module_with_redirection(wmodule_name, resolved_name);
		if (!module_entry)
		{
			module_entry = mapper.map_dependency(resolved_name.c_str());
			if (!module_entry)
				return false;
		}

		const auto module = module_entry.value();

		auto func_entry = from_rva<PIMAGE_THUNK_DATA>(import_descriptor->FirstThunk);
		auto lookup_entry = from_rva<PIMAGE_THUNK_DATA>(
			import_descriptor->OriginalFirstThunk ?
			import_descriptor->OriginalFirstThunk :
			import_descriptor->FirstThunk);

		for (; lookup_entry->u1.AddressOfData; ++func_entry, ++lookup_entry)
		{
			uintptr_t import_address;

			if (lookup_entry->u1.Ordinal & IMAGE_ORDINAL_FLAG)
			{
				import_address = uintptr_t(mapper.get_module_export(module, "", lookup_entry->u1.Ordinal & 0xFFFF));
			}
			else
			{
				const auto import_name = from_rva<PIMAGE_IMPORT_BY_NAME>(uint32_t(lookup_entry->u1.AddressOfData))->Name;
				import_address = uintptr_t(mapper.get_module_export(module, import_name, 0));
			}

			if (!import_address)
			{
				log_error("Failed to resolve import from module %ls.", wmodule_name.c_str());
				return false;
			}

			func_entry->u1.Function = import_address;
		}
	}

	return true;
}

ModuleMapper::ModuleMapper(DllMapper& mapper) :
	mapper(mapper)
{
}

std::optional<DllMapper::Module> ModuleMapper::map_module(const wchar_t* file_path)
{
	const auto file_buffer = util::read_file(file_path);
	if (file_buffer.empty())
	{
		log_error("Failed to read file %ls.", file_path);
		return std::nullopt;
	}

	const auto file_path_wstr = std::wstring(file_path);
	const auto last_slash = file_path_wstr.find_last_of(L"\\/");

	std::wstring module_name;
	if (last_slash == std::string::npos) module_name = file_path_wstr;
	else							     module_name = file_path_wstr.substr(last_slash + 1);

	if (!map_as_image(file_buffer))
	{
		log_error("Mapping image sections failed.");
		return std::nullopt;
	}

	const auto offset = (util::random_value(0, 5) + 1) * 0x1000;

	const auto remote_base = mapper.process.allocate(image_buffer.size() + offset, true) + offset;
	if (remote_base == offset)
	{
		log_error("Allocating memory for image failed.");
		return std::nullopt;
	}

	const auto delta = remote_base - nt_headers->OptionalHeader.ImageBase;
	if (!relocate(delta))
	{
		log_error("Relocating image failed.");
		return std::nullopt;
	}

	const auto entrypoint_rva = nt_headers->OptionalHeader.AddressOfEntryPoint;
	const auto entrypoint = entrypoint_rva ? (entrypoint_rva + remote_base) : 0;
	const auto headers_size = nt_headers->OptionalHeader.SizeOfHeaders;

	auto module = DllMapper::Module{ module_name, remote_base, image_buffer.size(), headers_size, entrypoint, image_buffer };
	mapper.temporary_mapped_modules.emplace_back(module);

	if (!resolve_imports())
	{
		log_error("Resolving imports failed.");
		return std::nullopt;
	}

	nt_headers = nullptr;
	local_base = nullptr;

	log_debug("Injected %ls successfuly to %p.", file_path, remote_base);

	return DllMapper::Module{ module_name, remote_base, image_buffer.size(), headers_size, entrypoint, std::move(image_buffer) };
}
