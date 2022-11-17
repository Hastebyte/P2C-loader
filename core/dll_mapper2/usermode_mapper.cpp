#include "usermode_mapper.h"

void __stdcall shellcode( MANUAL_MAPPING_DATA * mapping_data );

bool manual_map( HANDLE process, BYTE* module_data )
{
    IMAGE_NT_HEADERS *		nt_header = nullptr;
    IMAGE_OPTIONAL_HEADER * optional_header = nullptr;
    IMAGE_FILE_HEADER *		file_header = nullptr;


    if ( reinterpret_cast<IMAGE_DOS_HEADER*>( module_data )->e_magic != 0x5A4D )
    {
        printf( "Invalid file\n" );
        return false;
    }

    nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>( module_data + reinterpret_cast<IMAGE_DOS_HEADER*>( module_data )->e_lfanew );
    optional_header = &nt_header->OptionalHeader;
    file_header = &nt_header->FileHeader;

    // If the machine type is not the current file type we fail
    if ( file_header->Machine != IMAGE_FILE_MACHINE_AMD64 )
    {
        printf( "Invalid platform\n" );
        return false;
    }

    // Get the target base address to allocate memory in the target process
    // Try to load at image base of the old optional header, the size of the optional header image, commit = make , reserve it, execute read write to write the memory

    BYTE* target_base = reinterpret_cast<BYTE*>( VirtualAllocEx( process, reinterpret_cast<void*>( optional_header->ImageBase ), optional_header->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

    if ( !target_base )
    {
        // We can't allocate it, lets initialize it instead?
        // Forget the image base, just use nullptr, if this fails we cannot allocate memory in the target process.
        target_base = reinterpret_cast<BYTE*>( VirtualAllocEx( process, nullptr, optional_header->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE ) );

        if ( !target_base )
        {
            // We couldn't allocate memory.
            printf( "Memory allocation failed 0x%X\n", GetLastError( ) );
            return false;
        }
    }

    // Declare data to map
    MANUAL_MAPPING_DATA data{ 0 };

    // Declare function prototype
    data.pLoadLibraryA = LoadLibraryA;

    // Declare function prototype
    data.pGetProcAddress = reinterpret_cast<fnGetProcAddress>( GetProcAddress );

    // Get the section header
    auto* pSectionHeader = IMAGE_FIRST_SECTION( nt_header );

    // Loop the file header sections for section data, we only care about the raw data in here, it contains other data that is used after runtime which we dont care about.
    for ( UINT i = 0; i != file_header->NumberOfSections; ++i, ++pSectionHeader )
    {
        // If it's raw data
        if ( pSectionHeader->SizeOfRawData )
        {
            // Try to write our source data into the process, mapping.
            if ( !WriteProcessMemory( process, target_base + pSectionHeader->VirtualAddress, module_data + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr ) )
            {
                // We couldn't allocate memory

                printf( "Failed to allocate memory: 0x%x\n", GetLastError( ) );
                VirtualFreeEx( process, target_base, 0, MEM_RELEASE );
                return false;
            }
        }
    }

    // Copy our source data and our new data
    memcpy( module_data, &data, sizeof( data ) );
    WriteProcessMemory( process, target_base, module_data, 0x1000, nullptr );

    void* pshellcode = VirtualAllocEx( process, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE );

    if ( !pshellcode )
    {
        printf( "Memory allocation failed (1) (ex) 0x%X\n", GetLastError( ) );
        VirtualFreeEx( process, target_base, 0, MEM_RELEASE );
        return false;
    }

    WriteProcessMemory( process, pshellcode, shellcode, 0x1000, nullptr );

    HANDLE hThread = CreateRemoteThread( process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>( pshellcode ), target_base, 0, nullptr );

    if ( !hThread )
    {
        printf( "Thread creation failed 0x%X\n", GetLastError( ) );
        VirtualFreeEx( process, target_base, 0, MEM_RELEASE );
        VirtualFreeEx( process, pshellcode, 0, MEM_RELEASE );
        return false;
    }

    CloseHandle( hThread );

    HINSTANCE hCheck = NULL;

    while ( !hCheck )
    {
        MANUAL_MAPPING_DATA data_checked{ 0 };
        ReadProcessMemory( process, target_base, &data_checked, sizeof( data_checked ), nullptr );
        hCheck = data_checked.hMod;
        Sleep( 10 );
    }

    VirtualFreeEx( process, pshellcode, 0, MEM_RELEASE );

    return true;
}

void __stdcall shellcode( MANUAL_MAPPING_DATA * mapping_data )
{
    if ( !mapping_data )
        return;

    // Process base

    BYTE * pBase = reinterpret_cast<BYTE*>( mapping_data );

    // Optional data

    auto* pOptionalHeader = &reinterpret_cast<IMAGE_NT_HEADERS*>( pBase + reinterpret_cast<IMAGE_DOS_HEADER*>( mapping_data )->e_lfanew )->OptionalHeader;

    auto _LoadLibraryA = mapping_data->pLoadLibraryA;
    auto _GetProcAddress = mapping_data->pGetProcAddress;
    auto _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>( pBase + pOptionalHeader->AddressOfEntryPoint );

    BYTE * LocationDelta = pBase - pOptionalHeader->ImageBase;

    if ( LocationDelta )
    {
        if ( !pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size )
            return;

        auto* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>( pBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress );
        while ( pRelocData->VirtualAddress )
        {
            UINT AmountOfEntries = ( pRelocData->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION ) ) / sizeof( WORD );
            WORD * pRelativeInfo = reinterpret_cast<WORD*>( pRelocData + 1 );

            for ( UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo )
            {
                if ( RELOC_FLAG( *pRelativeInfo ) )
                {
                    UINT_PTR * pPatch = reinterpret_cast<UINT_PTR*>( pBase + pRelocData->VirtualAddress + ( ( *pRelativeInfo ) & 0xFFF ) );
                    *pPatch += reinterpret_cast<UINT_PTR>( LocationDelta );
                }
            }
            pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>( reinterpret_cast<BYTE*>( pRelocData ) + pRelocData->SizeOfBlock );
        }
    }

    if ( pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size )
    {
        auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>( pBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress );

        while ( pImportDescr->Name )
        {
            char* szMod = reinterpret_cast<char*>( pBase + pImportDescr->Name );
            HINSTANCE hDll = _LoadLibraryA( szMod );

            ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>( pBase + pImportDescr->OriginalFirstThunk );
            ULONG_PTR* pFuncRef = reinterpret_cast<ULONG_PTR*>( pBase + pImportDescr->FirstThunk );

            if ( !pThunkRef )
                pThunkRef = pFuncRef;

            for ( ; *pThunkRef; ++pThunkRef, ++pFuncRef )
            {
                if ( IMAGE_SNAP_BY_ORDINAL( *pThunkRef ) )
                {
                    *pFuncRef = _GetProcAddress( hDll, reinterpret_cast<char*>( *pThunkRef & 0xFFFF ) );
                } else
                {
                    auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>( pBase + ( *pThunkRef ) );
                    *pFuncRef = _GetProcAddress( hDll, pImport->Name );
                }
            }
            ++pImportDescr;
        }
    }

    if ( pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size )
    {
        auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>( pBase + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress );
        auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>( pTLS->AddressOfCallBacks );

        for ( ; pCallback && *pCallback; ++pCallback )
            ( *pCallback )( pBase, DLL_PROCESS_ATTACH, nullptr );
    }

    // Execute dll main

    _DllMain( pBase, DLL_PROCESS_ATTACH, nullptr );
    mapping_data->hMod = reinterpret_cast<HINSTANCE>( pBase );
}
