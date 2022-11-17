#include <QString>
#include <QTime>
#include <QDebug>
#include <QSettings>

#include <Windows.h>
#include <psapi.h>
#include <tlhelp32.h>

#include "xorstr.h"

QString random_string( unsigned int length );
bool is_elevated( );
int query_fastboot( );
int get_windows_build( );
DWORD get_process_id( LPCWSTR exe_name );
QString get_key_name( unsigned int virtual_key );
bool delete_usn_journal( const std::string& volume );
bool clean_other_caches( );
bool set_debug_privilege( );
