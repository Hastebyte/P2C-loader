#pragma once
#include <Windows.h>
#include <winternl.h>

namespace nt
{
	struct LDR_DATA
	{
		ULONG Length;
		UCHAR Initialized;
		PVOID SsHandle;
		LIST_ENTRY InLoadOrderModuleList;
	};

	struct LDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY InLoadOrderLinks;
		LIST_ENTRY InMemoryOrderLinks;
		LIST_ENTRY InInitializationOrderLinks;
		PVOID DllBase;
		PVOID EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
	};
}