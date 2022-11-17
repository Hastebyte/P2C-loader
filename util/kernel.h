#pragma once

#include <windows.h>
#include <tlhelp32.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <QDebug>

template<typename ... A>
uint64_t call_driver_control( void* control_function, const A ... arguments )
{
	if ( !control_function )
		return 0;

	using tFunction = uint64_t( __stdcall* )( A... );
	const auto control = static_cast<tFunction>( control_function );

	return control( arguments ... );
}

void* kernel_control_function( );
bool verify_hook( );
uint64_t spoof_drives( );


// check

/*
 * SYSTEM_SECUREBOOT_INFORMATION
The SYSTEM_SECUREBOOT_INFORMATION structure is what a successful call to ZwQuerySystemInformation or NtQuerySystemInformation produces in its output buffer when given the information class SystemSecureBootInformation (0x91).

Documentation Status
The SYSTEM_SECUREBOOT_INFORMATION structure is not documented.

Layout
The SYSTEM_SECUREBOOT_INFORMATION is 0x02 bytes in both 32-bit and 64-bit Windows.

Offset	Definition
0x00
BOOLEAN SecureBootEnabled;
0x01
BOOLEAN SecureBootCapable;
The function fills the structure with information that the ke
*/
