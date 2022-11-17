#include "kernel.h"

void* kernel_control_function( )
{
	//auto module_encrypted = xorstr( "win32u.dll" );
	//module_encrypted.crypt( );

	HMODULE hModule = LoadLibrary( L"win32u.dll" );

	if ( !hModule )
		return nullptr;

	// NtGdiDdDDINetDispQueryMiracastDisplayDeviceStatus
	// NtGdiDdDDINetDispQueryMiracastDisplayDeviceStatus
	//auto routine_encrypted = xorstr( "NtGdiDdDDINetDispQueryMiracastDisplayDeviceStatus" ); // NtDxgkRegisterVailProcess
	//auto routine_encrypted = xorstr( "NtTokenManagerCreateFlipObjectReturnTokenHandle" );
	//routine_encrypted.crypt( );

	return reinterpret_cast<void*>( GetProcAddress( hModule, "NtTokenManagerCreateFlipObjectReturnTokenHandle" ) );
}

bool verify_hook( )
{
	if ( !kernel_control_function( ) )
	{
		MessageBox( HWND_DESKTOP, L"Send developer this code: 001A", L"Spoof Error", MB_OK );
		return false;
	}

	uint64_t result = call_driver_control( kernel_control_function( ), 1338 );

	// qDebug( ) << "verify_hook" << QString::number( result, 16 );

	if ( result != 0x13371337 )
		return false;

	// if ( result != 0xABCFF133700 )
	// 	return false;

	return true;
}

uint64_t spoof_drives( )
{
    //call_driver_control( kernel_control_function( ), ID_DISABLE_SMART_ID );
    return 0;//call_driver_control( kernel_control_function( ), ID_CHANGE_HDD_SERIALS );
}
