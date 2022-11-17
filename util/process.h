#include <windows.h>
#include <stdio.h>
#include <string>

#include "shared.h"
#include "xorstr.h"

#include <QMessageBox>

#include "VMProtectSDK.h"
//#include "VirtualizerSDK.h"

BOOL CALLBACK enum_windows_proc( HWND hwnd, LPARAM lparam );
bool is_blacklisted_window_running( );
bool is_blacklisted_app_running( );
bool is_fiddler_running( );
bool is_game_running( );
