#include "injector.h"

BOOL inject_dll( DWORD dwPid, const char* szDLL )
{
	qDebug( ) << "Injecting" << szDLL << "into:" << dwPid;

	HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_CREATE_THREAD, true, dwPid );

	if ( hProcess == INVALID_HANDLE_VALUE )
	{
		qDebug( ) << "[!] Failed to call OpenProcess" << GetLastError( );
		// printf( "[!] Failed to call OpenProcess, GetLastError( ) = %d\n", GetLastError( ) );
		return FALSE;
	}

	LPVOID lpRemoteAddress = VirtualAllocEx( hProcess, NULL, strlen( szDLL ), MEM_COMMIT, PAGE_READWRITE );

	if( !lpRemoteAddress )
	{
		qDebug( ) << "[!] Failed to call VirtualAllocEx";
		// printf( "[!] Failed to call VirtualAllocEx Handle: %I64X, GetLastError( ) = %d\n", reinterpret_cast<unsigned __int64>( hProcess ), GetLastError( ) );
		return FALSE;
	}

	SIZE_T bytes;

	if( !WriteProcessMemory( hProcess, lpRemoteAddress, szDLL, strlen( szDLL ), &bytes ) )
	{
		qDebug( ) << "[!] Failed to call WriteProcessMemory( )";
		// printf( "[!] Failed to call WriteProcessMemory( ), GetLastError( ) = %d\n", GetLastError( ) );
		return FALSE;
	}

	FARPROC lpLoadLibrary = GetProcAddress( GetModuleHandleA( "KERNEL32.DLL" ), "LoadLibraryA" );

	if ( !lpLoadLibrary )
	{
		qDebug( ) << "[!] Failed to get address of LoadLibrar";
		// printf( "[!] Failed to get address of LoadLibrary\n" );
		return FALSE;
	}

	HANDLE hThread =
		CreateRemoteThread( hProcess, NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>( lpLoadLibrary ), lpRemoteAddress, NULL, nullptr );

	if( !hThread )
	{
		qDebug( ) << "[!] Failed to call CreateRemoteThread";
		// printf( "[!] Failed to call CreateRemoteThread, GetLastError( ) = %d\n", GetLastError( ) );
		return FALSE;
	}

	WaitForSingleObject( hThread, INFINITE );

	if( !VirtualFreeEx( hProcess, lpRemoteAddress, 0, MEM_RELEASE ) )
	{
		qDebug( ) << "[!] Failed to call VirtualFreeEx";
		// printf( "[!] Failed to call VirtualFreeEx, GetLastError( ) = %d\n", GetLastError( ) );
		return FALSE;
	}

	CloseHandle( hThread );

	return TRUE;
};
