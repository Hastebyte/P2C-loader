#pragma once
#include <Windows.h>

#define IOCTL_LDR_ALLOC		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LDR_FREE		CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LDR_HIDEMDL	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LDR_EXECUTE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LDR_MEMCPY	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

typedef struct _LOADER_ALLOC
{
	ULONG64 Size;
	struct
	{
		ULONG64 Mdl;
		ULONG64 Address;
	} Out;
} LOADER_ALLOC, *PLOADER_ALLOC;

typedef struct _LOADER_FREE
{
	ULONG64 Mdl;
} LOADER_FREE, *PLOADER_FREE;

typedef struct _LOADER_HIDEMDL
{
	ULONG64 Mdl;
} LOADER_HIDEMDL, *PLOADER_HIDEMDL;

typedef struct _LOADER_EXECUTE
{
	ULONG64 Entrypoint;
	ULONG64 Rcx;
} LOADER_EXECUTE, *PLOADER_EXECUTE;

typedef struct _LOADER_MEMCPY
{
	ULONG64 Source;
	ULONG64 Size;
	ULONG64 Destination;
} LOADER_MEMCPY, *PLOADER_MEMCPY;