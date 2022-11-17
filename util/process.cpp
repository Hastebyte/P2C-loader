#include "process.h"

BOOL CALLBACK enum_windows_proc( HWND hwnd, LPARAM lparam )
{
	UNREFERENCED_PARAMETER( lparam );

    return TRUE;

	//VIRTUALIZER_MUTATE_ONLY_START

    const std::string blacklisted_windows[] = { xorstr_( "Scylla" ), xorstr_( "x64_dbg" ), xorstr_( "Telerik Fiddler" ) };

	char buffer[1024] = {0};
	GetWindowTextA( hwnd, buffer, sizeof( buffer ) );

	const std::string window_name = buffer;

	for ( const std::string& blacklisted_window : blacklisted_windows )
	{
		if ( window_name.find( blacklisted_window ) != std::string::npos )
		{
			// qDebug( ) << "[-] blacklisted window" << QString( blacklisted_window.c_str( ) );
			QMessageBox::critical( nullptr, xorstr_( "Error" ), xorstr_( "Blacklisted tool is running" ) );
			TerminateProcess( GetCurrentProcess( ), 0 );
			return FALSE;
		}
	}

	//VIRTUALIZER_MUTATE_ONLY_END

	return TRUE;
}

bool is_blacklisted_window_running( )
{
	EnumWindows( enum_windows_proc, NULL );
	return false;
}

bool is_blacklisted_app_running( )
{
    return false;

	//VIRTUALIZER_MUTATE_ONLY_START

	const std::wstring blacklisted_apps[] = { L"windbg.exe", L"ida64.exe", L"wireshark.exe", L"x64dbg.exe" };

	for ( const std::wstring& app : blacklisted_apps )
	{
		if ( get_process_id( app.c_str( ) ) )
		{
			return true;
		}
	}

	//VIRTUALIZER_MUTATE_ONLY_END

	return false;
}

bool is_fiddler_running( )
{
    //VIRTUALIZER_MUTATE_ONLY_START

    HWND hwnd = FindWindowA( xorstr_( "WindowsForms10.Window.8.app.0.2bf8098_r6_ad1" ), NULL );

    if ( hwnd )
      return true;

    //VIRTUALIZER_MUTATE_ONLY_END

    return false;
}

bool is_game_running( )
{

    const std::wstring blacklisted_apps[] = { L"RainbowSix.exe", L"DayZ_x64.exe", L"RustClient.exe", L"Scum.exe", L"r5apex.exe", L"PUBGLite-Win64-Shipping.exe" };

	for ( const std::wstring& app : blacklisted_apps )
	{
		if ( get_process_id( app.c_str( ) ) )
		{
			return true;
		}
	}

	return false;
}
