#include "dll_mapper.hpp"
#include "nt_structs.hpp"
#include "module_mapper.hpp"
#include "redirection.hpp"
#include "util.hpp"
#include <filesystem>

DllMapper::Module DllMapper::read_module(const Module& module)
{
	if (module.buffer.empty())
	{
		std::vector<uint8_t> buffer(module.size);
		process.read_buffer(module.base_address, buffer.data(), module.size);

		return Module{ module.name, module.base_address, module.size, module.headers_size, module.entrypoint, std::move(buffer) };
	}

	return module;
}

uintptr_t DllMapper::get_forwarded_export(const Module& module, uintptr_t forwarder)
{
	const auto forward_string = std::string((const char*)forwarder);

	const auto dot = forward_string.find('.');
	if (dot == std::string::npos)
	{
		log_error("Invalid forwarder string %s.", forward_string.c_str());
		return 0;
	}

	const auto module_name = forward_string.substr(0, dot) + ".dll";
	const auto wmodule_name = std::wstring(module_name.begin(), module_name.end());
	const auto function_name = forward_string.substr(dot + 1);

	const auto resolved_module = redirection::resolve(wmodule_name, module.name);
	if (!resolved_module)
	{
		log_error("Failed to resolve redirection %ls.", wmodule_name.c_str());
		return 0;
	}

	const auto resolved_name = resolved_module.value();
	auto forwarded_module = get_process_module(resolved_name);
	if (!forwarded_module)
	{
		forwarded_module = map_dependency(resolved_name.c_str());
		if (!forwarded_module)
		{
			log_error("Failed to map dependency %ls.", resolved_name.c_str());
			return 0;
		}
	}

	return get_module_export(forwarded_module.value(), function_name, 0);
}

uintptr_t DllMapper::get_module_export(const Module& module, const std::string& name, uint16_t ordinal)
{
	const auto local_base = module.buffer.data();
	const auto nt_headers = util::get_nt_headers(local_base);
	if (!nt_headers)
	{
		log_error("Module %ls has invalid PE headers.", module.name.c_str());
		return 0;
	}

	const auto& export_directory = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (!export_directory.Size || !export_directory.VirtualAddress)
	{
		log_error("Module %ls doesn't have export directory.", module.name.c_str());
		return 0;
	}

	const auto exports   = PIMAGE_EXPORT_DIRECTORY(local_base + export_directory.VirtualAddress);
	const auto functions = (uint32_t*)(local_base + exports->AddressOfFunctions);
	const auto names     = (uint32_t*)(local_base + exports->AddressOfNames);
	const auto ords      = (uint16_t*)(local_base + exports->AddressOfNameOrdinals);

	const auto use_ordinal = name.empty();

	if (use_ordinal)
	{
		for (size_t i = 0; i < exports->NumberOfFunctions; ++i)
		{
			if (i + exports->Base == ordinal)
			{
				const auto address = uintptr_t(local_base + functions[i]);
				const auto is_forwarded =
					address >= uintptr_t(exports) &&
					address < uintptr_t(exports) + export_directory.Size;

				return is_forwarded ? get_forwarded_export(module, address) : (address - uintptr_t(local_base) + module.base_address);
			}
		}

		log_error("Export %s (%d) in module %ls not found.", name.c_str(), ordinal, module.name.c_str());
		return 0;
	}

	for (size_t i = 0; i < exports->NumberOfFunctions; ++i)
	{
		const auto current_ord = ords[i];
		auto found = false;

		if (use_ordinal && current_ord == ordinal)
		{
			found = true;
		}
		else if (!use_ordinal && i < exports->NumberOfNames)
		{
			const auto current_name = (const char*)(local_base + names[i]);
			if (name == current_name)
				found = true;
		}

		if (found)
		{
			const auto address = uintptr_t(local_base + functions[current_ord]);
			const auto is_forwarded =
				address >= uintptr_t(exports) &&
				address <  uintptr_t(exports) + export_directory.Size;

			return is_forwarded ? get_forwarded_export(module, address) : (address - uintptr_t(local_base) + module.base_address);
		}
	}

	log_error("Export %s (%d) in module %ls not found.", name.c_str(), ordinal, module.name.c_str());
	return 0;
}

std::optional<DllMapper::Module> DllMapper::get_process_module(const std::wstring& name)
{
	const auto name_cstr = name.c_str();
	const auto find_module = [&](const std::vector<Module>& modules) -> std::optional<Module>
	{
		for (const auto& module : modules)
		{
			if (!_wcsicmp(module.name.c_str(), name_cstr))
				return read_module(module);
		}

		return std::nullopt;
	};

	if (const auto module = find_module(normal_modules); module)
		return module;

	if (const auto module = find_module(mapped_modules); module)
		return module;

	if (const auto module = find_module(temporary_mapped_modules); module)
		return module;

	return std::nullopt;
}

std::optional<DllMapper::Module> DllMapper::get_process_module_with_redirection(const std::wstring& name, std::wstring& resolved_name)
{
	resolved_name = name;

	if (const auto module = get_process_module(name); module)
		return module;

	if (const auto redirected = redirection::resolve(name); redirected)
	{
		resolved_name = redirected.value();
		return get_process_module(resolved_name);
	}

	return std::nullopt;
}

std::optional<DllMapper::Module> DllMapper::map_module_internal(const wchar_t* file_path)
{
	if (!std::filesystem::exists(file_path))
	{
		log_error("File %ls doesn't exist.", file_path);
		return std::nullopt;
	}

	//log_debug("Mapping module %ls.", file_path);

	ModuleMapper module_mapper(*this);
	const auto mapped_module = module_mapper.map_module(file_path);
	if (mapped_module)
		mapped_modules.emplace_back(mapped_module.value());

	return mapped_module;
}

std::optional<DllMapper::Module> DllMapper::map_dependency(const wchar_t* file_name)
{
	wchar_t system_directory[512];
	if (!GetSystemDirectoryW(system_directory, UINT(std::size(system_directory))))
	{
		log_error("Failed to get system directory.");
		return std::nullopt;
	}

	const auto file_path = std::wstring(system_directory) + L"\\" + file_name;

	return map_module_internal(file_path.c_str());
}

void DllMapper::clear_headers_and_write_images()
{
	for (auto& module : mapped_modules)
	{
		//for (size_t i = 0; i < module.headers_size; ++i)
		//	module.buffer[i] = uint8_t(util::random_value(0, 256));

		process.write_buffer(module.base_address, module.buffer.data(), module.buffer.size());
	}
}

std::vector<uint8_t> DllMapper::create_execution_shellcode()
{
	std::vector<uint8_t> shellcode;

	const uint8_t prologue[] =
	{
																// shellcode_start:
		0xF0, 0x0F, 0xBA, 0x2D, 0x03, 0x00, 0x00, 0x00, 0x00,	// lock bts dword [rel critical_section], 0
		0x73, 0x05,												// jnc	execute_rest
		0xC3,													// ret
		0x00, 0x00, 0x00, 0x00,									// critical_section: dd 0
																// execute_rest:
		0x57,													// push rdi
		0x48, 0x8D, 0x3D, 0xE8, 0xFF, 0xFF, 0xFF,				// lea	rdi, [rel shellcode_start]
		0xC6, 0x07, 0xC3,										// mov	byte [rdi], 0xC3
		0x51,													// push rcx
		0x52,													// push rdx
		0x41, 0x50,												// push r8
		0x41, 0x51,												// push r9
		0x48, 0x83, 0xEC, 0x20,									// sub	rsp, 0x20
	};

	for (const auto byte : prologue)
		shellcode.emplace_back(byte);

	for (const auto& module : mapped_modules)
	{
		if (!module.entrypoint)
			continue;

		uint8_t entrypoint_shellcode[] =
		{
			0x48, 0x8B, 0x0D, 0x28, 0x00, 0x00, 0x00,			// mov	rcx, qword [rel base_address]
			0xBA, 0x01, 0x00, 0x00, 0x00,						// mov	rdx, 1
			0x4D, 0x31, 0xC0,									// xor	r8, r8
			0xFF, 0x15, 0x12, 0x00, 0x00, 0x00,					// call [rel entrypoint]
			0x31, 0xC0,											// xor	eax, eax
			0x48, 0x89, 0x05, 0x09, 0x00, 0x00, 0x00,			// mov	qword [rel entrypoint], rax
			0x48, 0x89, 0x05, 0x0A, 0x00, 0x00, 0x00,			// mov	qword [rel base_address], rax
			0xEB, 0x10,											// jmp continue
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// entrypoint: dq 0
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// base_address: dq 0
																// continue:
		};
		
		*(uint64_t*)(entrypoint_shellcode + sizeof(entrypoint_shellcode) - 8)  = module.base_address;
		*(uint64_t*)(entrypoint_shellcode + sizeof(entrypoint_shellcode) - 16) = module.entrypoint;

		for (const auto byte : entrypoint_shellcode)
			shellcode.emplace_back(byte);
	}

	const uint8_t epilogue[] =
	{
		0x48, 0x83, 0xC4, 0x20,						// add	rsp, 0x20
		0x41, 0x59,									// pop	r9
		0x41, 0x58,									// pop	r8
		0x5A,										// pop	rdx
		0x48, 0x8D, 0x0D, 0x0A, 0x00, 0x00, 0x00,	// lea	rcx, [rel clear_point]
		0x48, 0x29, 0xF9,							// sub	rcx, rdi
		0xB8, 0xC3, 0x00, 0x00, 0x00,				// mov	eax, 0xC3
		0xF3, 0xAA,									// rep	stosb
													// clear_point:
		0x59,										// pop	rcx
		0x5F,										// pop	rdi
		0xC3,										// ret
	};

	for (const auto byte : epilogue)
		shellcode.emplace_back(byte);

	return shellcode;
}

DllMapper::DllMapper(Process target_process)
	: process(std::move(target_process))
{
	const auto peb  = process.read<PEB>(process.get_peb_address());
	const auto ldr  = process.read<nt::LDR_DATA>(uintptr_t(peb.Ldr));
	const auto head = uintptr_t(peb.Ldr) + offsetof(nt::LDR_DATA, InLoadOrderModuleList);

	for (auto link = uintptr_t(ldr.InLoadOrderModuleList.Flink); link != head; link = process.read<uintptr_t>(link))
	{
		const auto record = process.read<nt::LDR_DATA_TABLE_ENTRY>(
			uintptr_t(CONTAINING_RECORD(link, nt::LDR_DATA_TABLE_ENTRY, InLoadOrderLinks))
		);

		std::wstring name;
		name.resize(record.BaseDllName.Length / sizeof(wchar_t));
		process.read_buffer(uintptr_t(record.BaseDllName.Buffer), name.data(), record.BaseDllName.Length);

		normal_modules.emplace_back(Module{ name, uintptr_t(record.DllBase), record.SizeOfImage });
	}
}

bool DllMapper::map_module(const wchar_t* file_name_or_path)
{
	wchar_t file_path[1024];
	if (!GetFullPathNameW(file_name_or_path, DWORD(std::size(file_path)), file_path, nullptr))
	{
		log_error("Failed to get file path of %ls.", file_name_or_path);
		return false;
	}

	mapped_modules.clear();

	if (!map_module_internal(file_path).has_value())
		return false;

	clear_headers_and_write_images();

	const auto shellcode = create_execution_shellcode();
	const auto shellcode_address = process.allocate(shellcode.size(), true);
	process.write_buffer(shellcode_address, shellcode.data(), shellcode.size());

	process.execute(shellcode_address, 0);

	return true;
}
