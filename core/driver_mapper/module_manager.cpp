#include "module_manager.hpp"
#include <ntstatus.h>
#include <memory>

#include "../../util/xorstr.h"

#define  WIN32_LEAN_AND_MEAN
#define  WIN32_NO_STATUS
#define  NOMINMAX
#include <Windows.h>
#include <winternl.h>

namespace nt
{
	struct SYSTEM_MODULE_ENTRY
	{
		HANDLE	Section;
		PVOID	MappedBase;
		PVOID	ImageBase;
		ULONG	ImageSize;
		ULONG	Flags;
		USHORT	LoadOrderIndex;
		USHORT	InitOrderIndex;
		USHORT	LoadCount;
		USHORT	OffsetToFileName;
		UCHAR	FullPathName[256];
	};

	struct SYSTEM_MODULE_INFORMATION
	{
		ULONG				Count;
		SYSTEM_MODULE_ENTRY Module[1];
	};
}

void* ModuleManager::Module::get_local_va() noexcept
{
	if (!local_va)
	{
		local_va = (void*)LoadLibraryExA(name.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
		if (!local_va)
		{
			const auto full_path = std::string(xorstr_(R"(C:\Windows\System32\Drivers\)")) + name;
			local_va = (void*)LoadLibraryExA(full_path.c_str(), nullptr, DONT_RESOLVE_DLL_REFERENCES);
		}

		if (local_va)
		{
			std::printf(xorstr_("Loaded module: %s (kernel VA %p) (user VA %p).\n"), 
						name.c_str(), kernel_va, local_va);
		}
		else
		{
			std::printf(xorstr_("Failed loading module: %s.\n"), name.c_str());
		}
	}

	return local_va;
}

ModuleManager::Module::Module(std::string name, void* kernel_va) noexcept :
	kernel_va(kernel_va), local_va(nullptr), name(std::move(name))
{
}

ModuleManager::Module::~Module() noexcept
{
	if (local_va)
		FreeLibrary(HMODULE(local_va));

	local_va = nullptr;
}

const std::string& ModuleManager::Module::get_name() const noexcept
{
	return name;
}

void* ModuleManager::Module::get_export_by_name(const char* export_name) noexcept
{
	const auto hmodule = HMODULE(get_local_va());
	if (!hmodule)
		return nullptr;

	const auto address = uintptr_t(GetProcAddress(hmodule, export_name));
	if (!address)
		return nullptr;

	return (void*)(uintptr_t(kernel_va) + (address - uintptr_t(hmodule)));
}

void* ModuleManager::Module::get_export_by_ordinal(const uint16_t ordinal) noexcept
{
	return get_export_by_name((const char*)ordinal);
}

ModuleManager::ModuleManager()
{
	auto req_size = ULONG{ 0 };
	auto buf_size = ULONG{ 0 };
	std::unique_ptr<uint8_t[]> buffer;

	NTSTATUS status;
	while ((status = NtQuerySystemInformation(
		SYSTEM_INFORMATION_CLASS(11), 
		buffer.get(),
		buf_size,
		&req_size)) == STATUS_INFO_LENGTH_MISMATCH)
	{
		buf_size = req_size;
		buffer.reset(new uint8_t[buf_size]);
	}

	if (!NT_SUCCESS(status))
	{
		const auto string = xorstr_("NtQuerySystemInformation failed. NTSTATUS: %X.\n");

		std::printf(string, status);
		throw ModuleException();
	}

	const auto module_info = (nt::SYSTEM_MODULE_INFORMATION*)buffer.get();
	for (size_t i = 0; i < module_info->Count; ++i)
	{
		const auto& module_entry = module_info->Module[i];
		const auto  name = std::string((const char*)module_entry.FullPathName + module_entry.OffsetToFileName);

		modules.emplace_back(name, module_entry.ImageBase);
	}
}

ModuleManager::Module* ModuleManager::find_module(const std::string& name) noexcept
{
	for (auto& entry : modules)
	{
		if (name == entry.get_name())
			return &entry;
	}

	return nullptr;
}
