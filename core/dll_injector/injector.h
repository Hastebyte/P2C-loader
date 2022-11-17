#ifndef LOADER_H
#define LOADER_H

#include <Windows.h>
#include <string>
#include <TlHelp32.h>
#include <QDebug>

/*
DWORD GetProcessByName( const char* lpExeName )
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if ( hSnapShot == INVALID_HANDLE_VALUE )
	{
		#ifdef _DEBUG
			printf( "[!] Failed to call CreateToolhelp32Snapshot, GetLastError( ) = %d\n", GetLastError( ) );
		#endif

		return false;
	}

	PROCESSENTRY32 pe = { 0 };
	pe.dwSize = sizeof( PROCESSENTRY32 );

	for ( BOOL	success = Process32First( hSnapShot, &pe );
				success == TRUE;
				success = Process32Next( hSnapShot, &pe ) )
		{
			if ( strcmp( lpExeName, pe.szExeFile ) == 0 )
			{
				CloseHandle( hSnapShot );
				return pe.th32ProcessID;
			}
		}

	CloseHandle( hSnapShot );
	return NULL;
}
*/

BOOL inject_dll( DWORD dwPid, const char* szDLL );


#endif
