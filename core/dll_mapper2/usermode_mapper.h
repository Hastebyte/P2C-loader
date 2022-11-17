#include <windows.h>
#include <iostream>
#include <fstream>
#include <tlhelp32.h>

#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)
#define RELOC_FLAG RELOC_FLAG64


using fnLoadLibraryA	= HINSTANCE	( WINAPI* )( const char* lpLibFilename );
using fnGetProcAddress	= UINT_PTR	( WINAPI* )( HINSTANCE hModule, const char* lpProcName );
using f_DLL_ENTRY_POINT = BOOL		( WINAPI* )( void* hDll, DWORD dwReason, void* pReserved );

struct MANUAL_MAPPING_DATA
{
    fnLoadLibraryA		pLoadLibraryA;
    fnGetProcAddress	pGetProcAddress;
    HINSTANCE			hMod;
};

bool manual_map( HANDLE hProc, BYTE* pSourceData );
