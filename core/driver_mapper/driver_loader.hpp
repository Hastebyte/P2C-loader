#pragma once

#include "windows.h"

#pragma comment( lib, "advapi32.lib" )

namespace driver_loader
{
	bool initialize();

	bool load(const wchar_t* driver_name);
	bool unload(const wchar_t* driver_name);
}
