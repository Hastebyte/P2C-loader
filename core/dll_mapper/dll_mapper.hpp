#pragma once
#include "mapper_process.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <Windows.h>
#include <winternl.h>

class DllMapper
{
	friend class ModuleMapper;

	struct Module
	{
		std::wstring		 name;
		uintptr_t			 base_address;
		size_t				 size;

		// 0 for not mapped loaded modules
		size_t				 headers_size;
		uintptr_t			 entrypoint;
		std::vector<uint8_t> buffer;
	};

	Process				process;
	std::vector<Module> normal_modules;
	std::vector<Module> mapped_modules;
	std::vector<Module> temporary_mapped_modules;

	Module read_module(const Module& module);
	uintptr_t get_forwarded_export(const Module& module, uintptr_t forwarder);
	uintptr_t get_module_export(const Module& module, const std::string& name, uint16_t ordinal);

	std::optional<Module> get_process_module(const std::wstring& name);
	std::optional<Module> get_process_module_with_redirection(const std::wstring& name, std::wstring& resolved_name);

	std::optional<Module> map_module_internal(const wchar_t* file_path);
	std::optional<Module> map_dependency(const wchar_t* file_name);

	void clear_headers_and_write_images();
	std::vector<uint8_t> create_execution_shellcode();

public:
	DllMapper(Process target_process);

	DllMapper(DllMapper&&) = delete;
	DllMapper(const DllMapper&) = delete;
	DllMapper& operator=(DllMapper&&) = delete;
	DllMapper& operator=(const DllMapper&) = delete;

	bool map_module(const wchar_t* file_name_or_path);
};
