#include "mapper_process.hpp"
#include <Windows.h>
#include <winternl.h>
#include <TlHelp32.h>

#pragma comment(lib, "ntdll")

Process::Process(void* process_handle) 
	: process_handle(process_handle)
{
}

std::optional<Process> Process::find(const wchar_t* process_name)
{
	const auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return std::nullopt;

	PROCESSENTRY32W process_entry{ sizeof(PROCESSENTRY32W) };

	if (Process32FirstW(snapshot, &process_entry))
	{
		do
		{
			if (!wcscmp(process_name, process_entry.szExeFile))
			{
				const auto pid = process_entry.th32ProcessID;
				CloseHandle(snapshot);

				const auto process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
				if (!process_handle)
					return std::nullopt;

				return std::make_optional(Process(process_handle));
			}
		} while (Process32NextW(snapshot, &process_entry));
	}

	CloseHandle(snapshot);
	return std::nullopt;
}

Process::Process(Process&& process) noexcept
{
	process_handle = process.process_handle;
	process.process_handle = nullptr;
}

Process& Process::operator=(Process&& process) noexcept
{
	if (this != &process)
	{
		CloseHandle(process_handle);

		process_handle = process.process_handle;
		process.process_handle = nullptr;
	}

	return *this;
}

Process::~Process()
{
	if (process_handle)
		CloseHandle(process_handle);
}

bool Process::read_buffer(uintptr_t address, void* buffer, size_t size)
{
	return ReadProcessMemory(process_handle, (void*)address, buffer, size, nullptr);
}

bool Process::write_buffer(uintptr_t address, const void* buffer, size_t size)
{
	return WriteProcessMemory(process_handle, (void*)address, buffer, size, nullptr);
}

uintptr_t Process::get_peb_address()
{
	PROCESS_BASIC_INFORMATION pbi;
	ULONG required_length;

	if (!NT_SUCCESS(NtQueryInformationProcess(process_handle, ProcessBasicInformation, &pbi, sizeof(pbi), &required_length)))
		return 0;

	return uintptr_t(pbi.PebBaseAddress);
}

uintptr_t Process::allocate(size_t size, bool execute)
{
	return uintptr_t(VirtualAllocEx(process_handle, nullptr, size, MEM_COMMIT | MEM_RESERVE, execute ? PAGE_EXECUTE_READWRITE : PAGE_READWRITE));
}

bool Process::free(uintptr_t address)
{
	return VirtualFreeEx(process_handle, (void*)address, 0, MEM_RELEASE);
}

bool Process::execute(uintptr_t address, uintptr_t parameter)
{
	const auto handle = CreateRemoteThread(process_handle, nullptr, 0, LPTHREAD_START_ROUTINE(address), (void*)parameter, 0, nullptr);
	const auto valid = handle != nullptr;

	if (valid)
		CloseHandle(handle);

	return valid;
}
