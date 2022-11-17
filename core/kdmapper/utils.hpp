#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "nt.hpp"
#include "..\..\util\xorstr.h"


namespace utils
{
	bool ReadFileToMemory(const std::string& file_path, std::vector<uint8_t>* out_buffer);
	bool CreateFileFromMemory(const std::string& desired_file_path, const char* address, size_t size);
	uint64_t GetKernelModuleAddress(const std::string& module_name);

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
}
