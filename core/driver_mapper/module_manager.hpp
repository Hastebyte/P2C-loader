#pragma once

#include <string>
#include <vector>
#include <cstdint>

#pragma comment( lib, "ntdll.lib" )

class ModuleManager
{
public:
	class ModuleException : std::exception
	{
		using std::exception::exception;
	};

	class Module
	{
		void*		kernel_va;
		void*		local_va;
		std::string name;

		void* get_local_va() noexcept;

	public:
		Module(std::string name, void* kernel_va) noexcept;
		~Module() noexcept;

		Module(Module&&) = default;
		Module(const Module&) = delete;
		Module& operator=(Module&&) = default;
		Module& operator=(const Module&) = delete;

		const std::string& get_name() const noexcept;
		void* get_export_by_name(const char* export_name) noexcept;
		void* get_export_by_ordinal(uint16_t ordinal) noexcept;
	};

private:
	std::vector<Module> modules;

public:
	ModuleManager();
	~ModuleManager() = default;

	ModuleManager(ModuleManager&&) = default;
	ModuleManager(const ModuleManager&) = delete;
	ModuleManager& operator=(ModuleManager&&) = default;
	ModuleManager& operator=(const ModuleManager&) = delete;

	Module* find_module(const std::string& name) noexcept;
};
