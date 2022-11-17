#include "driver_loader.hpp"

#include <Windows.h>
#include <winternl.h>
#include <Shlwapi.h>
#include <cstdint>
#include <string>

#include "../../util/xorstr.h"

#pragma comment(lib, "Shlwapi.lib")

static NTSTATUS(NTAPI *NtLoadDriver)(PUNICODE_STRING);
static NTSTATUS(NTAPI *NtUnloadDriver)(PUNICODE_STRING);
static NTSTATUS(NTAPI *RtlAdjustPrivilege)(ULONG, BOOLEAN, BOOLEAN, PBOOLEAN);

static bool get_load_driver_privillege()
{
	BOOLEAN enabled;
	return !RtlAdjustPrivilege(10, true, false, &enabled) || enabled;
}

static bool does_key_exist(const wchar_t* driver_name)
{
	auto key		= HKEY{ };
	const auto path	= std::wstring(xorstr_(LR"(System\CurrentControlSet\Services\)")) + driver_name;
	if (path.empty())
		return false;

	const auto status = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_ALL_ACCESS, &key);
	RegCloseKey(key);

	return status != ERROR_FILE_NOT_FOUND;
}

static bool remove_key(const wchar_t* driver_name)
{
	const auto path = std::wstring(xorstr_(LR"(System\CurrentControlSet\Services\)")) + driver_name;
	if (path.empty())
		return true;

	auto status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, path.c_str());
	if (!status || status == ERROR_FILE_NOT_FOUND)
		return true;

	status = SHDeleteKeyW(HKEY_LOCAL_MACHINE, path.c_str());
	if (!status || status == ERROR_FILE_NOT_FOUND)
		return true;

	status = RegDeleteKeyW(HKEY_LOCAL_MACHINE, path.c_str());
	return !status || status == ERROR_FILE_NOT_FOUND;
}

static bool add_key(const wchar_t* driver_name)
{
	remove_key(driver_name);

	auto key		= HKEY{ };
	const auto path = std::wstring(xorstr_(LR"(System\CurrentControlSet\Services\)")) + driver_name;

	if (path.empty())
	{
		return false;
	}

	auto status = RegCreateKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, nullptr);

	if (status)
	{
		return false;
	}

	const auto write_string = [=](const wchar_t* name, const std::wstring& data) {
		return RegSetValueExW(key, name, 0, REG_EXPAND_SZ, (uint8_t*)data.c_str(), (DWORD)data.size() * sizeof(wchar_t));
	};
	const auto write_dword = [=](const wchar_t* name, DWORD data) {
		return RegSetValueExW(key, name, 0, REG_DWORD, (uint8_t*)&data, (DWORD)sizeof(DWORD));
	};

	status |= write_string(xorstr_(L"ImagePath"), 
		std::wstring(xorstr_(LR"(\SystemRoot\System32\drivers\)")) + driver_name + xorstr_(L".sys"));

	status |= write_dword(xorstr_(L"Type"), 1);
	status |= write_dword(xorstr_(L"ErrorControl"), 1);
	status |= write_dword(xorstr_(L"Start"), 3);

	RegCloseKey(key);
	if (status)
	{
		remove_key(driver_name);
		return false;
	}

	return true;
}

bool driver_loader::initialize()
{
	const auto ntdll	= GetModuleHandleW(xorstr_(L"ntdll"));

	NtLoadDriver		= (decltype(NtLoadDriver))GetProcAddress(ntdll, xorstr_("NtLoadDriver"));
	NtUnloadDriver		= (decltype(NtUnloadDriver))GetProcAddress(ntdll, xorstr_("NtUnloadDriver"));
	RtlAdjustPrivilege	= (decltype(RtlAdjustPrivilege))GetProcAddress(ntdll, xorstr_("RtlAdjustPrivilege"));

	return get_load_driver_privillege();
}

bool driver_loader::load(const wchar_t* driver_name)
{
	if (!std::wcslen(driver_name))
		return false;

	unload(driver_name);
	if (!add_key(driver_name))
	{
		std::printf(xorstr_("Adding registry key failed.\n"));
		return false;
	}

	const auto path = std::wstring(xorstr_(LR"(\Registry\Machine\System\CurrentControlSet\Services\)")) + driver_name;
	if (path.empty())
		return false;

	UNICODE_STRING reg_path;
	reg_path.Buffer			= (wchar_t*)path.c_str();
	reg_path.Length			= (USHORT)(path.size() + 0) * 2;
	reg_path.MaximumLength	= (USHORT)(path.size() + 1) * 2;

	const auto status = NtLoadDriver(&reg_path);
	if (status)
	{
		std::printf(xorstr_("NtLoadDriver failed. NTSTATUS: %x.\n"), status);

		remove_key(driver_name);
		return false;
	}

	return true;
}

bool driver_loader::unload(const wchar_t* driver_name)
{
	if (!std::wcslen(driver_name))
		return false;

	if (!does_key_exist(driver_name))
		add_key(driver_name);

	const auto path = std::wstring(xorstr_(LR"(\Registry\Machine\System\CurrentControlSet\Services\)")) + driver_name;
	if (path.empty())
		return false;

	UNICODE_STRING reg_path;
	reg_path.Buffer			= (wchar_t*)path.c_str();
	reg_path.Length			= (USHORT)(path.size() + 0) * 2;
	reg_path.MaximumLength	= (USHORT)(path.size() + 1) * 2;

	const auto status = NtUnloadDriver(&reg_path);
	remove_key(driver_name);

	return !status;
}
