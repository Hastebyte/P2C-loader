#include "driver.hpp"
#include "driver_loader.hpp"
#include "driver_binary.hpp"
#include "driver_internal.hpp"
#include <winternl.h>
#include <fstream>

#include "../../util/xorstr.h"

static std::wstring driver_name;
static std::string  get_driver_path()
{
	wchar_t system_directory[2048];
	GetSystemDirectoryW(system_directory, 2048);

	const auto path = std::wstring(system_directory) + xorstr_(LR"(\drivers\)") + driver_name + xorstr_(L".sys");
	return std::string(path.begin(), path.end());
}

bool Driver::load_driver()
{
	if (!std::wcslen(driver_name.c_str()))
		return false;

	const auto path = get_driver_path();
	std::ofstream file(path, std::ios_base::binary);

	for (auto& byte : driver_binary)
		byte ^= driver_xor_key;

	file.write((char*)driver_binary, sizeof(driver_binary));

	for (auto& byte : driver_binary)
		byte = 0x00;

	file.close();

	if (!driver_loader::load(driver_name.c_str()))
	{
		std::printf(xorstr_("Loading driver failed.\n"));

		std::remove(path.c_str());
		return false;
	}

	device_handle = CreateFileW(
		xorstr_(L"\\\\.\\pcihid"), 
		FILE_ALL_ACCESS, 
		FILE_SHARE_READ, 
		nullptr, 
		FILE_OPEN, 
        FILE_FLAG_OVERLAPPED,
		nullptr
	);

	if (!device_handle || device_handle == INVALID_HANDLE_VALUE)
	{
		std::printf(xorstr_("Opening handle to device failed.\n"));

		device_handle = nullptr;

		driver_loader::unload(driver_name.c_str());
		std::remove(path.c_str());

		return false;
	}

	return true;
}

Driver::Driver()
{
	device_handle = nullptr;
	driver_name   = xorstr_(L"pcchid");

	if (!load_driver())
		throw DriverException();

	std::printf(xorstr_("Opened handle to Loader device: %p.\n"), device_handle);
}

Driver::~Driver()
{
	if (device_handle)
	{
		CloseHandle(device_handle);
		driver_loader::unload(driver_name.c_str());

		const auto path = get_driver_path();
		std::remove(path.c_str());
	}
}

Driver::Allocation Driver::allocate(const size_t size) const noexcept
{
	LOADER_ALLOC buffer;
	buffer.Size = size;

	DWORD bytes_returned;
	const bool success = DeviceIoControl(
		device_handle,
		IOCTL_LDR_ALLOC,
		(void*)&buffer,
		sizeof(buffer),
		(void*)&buffer,
		sizeof(buffer),
		&bytes_returned,
		nullptr
	);

	Allocation out_allocation{};
	if (success)
	{
		out_allocation.kernel_va   = (void*)buffer.Out.Address;
		out_allocation.mdl_address = (void*)buffer.Out.Mdl;
	}

	return out_allocation;
}

bool Driver::free(const Allocation& allocation) const noexcept
{
	LOADER_FREE buffer;
	buffer.Mdl = ULONG64(allocation.mdl_address);

	DWORD bytes_returned;
	return DeviceIoControl(
		device_handle,
		IOCTL_LDR_FREE,
		(void*)&buffer,
		sizeof(buffer),
		(void*)&buffer,
		sizeof(buffer),
		&bytes_returned,
		nullptr
	);
}

bool Driver::hide(const Allocation& allocation) const noexcept
{
	LOADER_HIDEMDL buffer;
	buffer.Mdl = ULONG64(allocation.mdl_address);

	DWORD bytes_returned;
	return DeviceIoControl(
		device_handle,
		IOCTL_LDR_HIDEMDL,
		(void*)&buffer,
		sizeof(buffer),
		(void*)&buffer,
		sizeof(buffer),
		&bytes_returned,
		nullptr
	);
}

bool Driver::memcpy(void* source, void* destination, const size_t size) const noexcept
{
	LOADER_MEMCPY buffer;
	buffer.Source		= ULONG64(source);
	buffer.Destination	= ULONG64(destination);
	buffer.Size			= size;

	DWORD bytes_returned;
	return DeviceIoControl(
		device_handle,
		IOCTL_LDR_MEMCPY,
		(void*)&buffer,
		sizeof(buffer),
		(void*)&buffer,
		sizeof(buffer),
		&bytes_returned,
		nullptr
	);
}

#include <QDebug>

bool Driver::execute(void* entry, void* argument) const noexcept
{
	LOADER_EXECUTE buffer;
	buffer.Entrypoint	= ULONG64(entry);
	buffer.Rcx			= ULONG64(argument);

   // qDebug( ) <<  "[-] executing DeviceIoControl";

	DWORD bytes_returned;
    BOOL ret = DeviceIoControl(
		device_handle,
		IOCTL_LDR_EXECUTE,
		(void*)&buffer,
		sizeof(buffer),
		(void*)&buffer,
		sizeof(buffer),
		&bytes_returned,
		nullptr
	);

  //  qDebug( ) <<  "[-] executed";

    return ret;
}
