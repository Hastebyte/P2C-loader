#include "shared.h"

QString random_string( unsigned int length )
{
	qsrand( static_cast<unsigned int>( QTime::currentTime( ).msec( ) ) );

	const QString character_set( xorstr_( "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" ) );

	QString result;

	for ( unsigned int i = 0; i < length; ++i )
	{
	   int index = qrand( ) % character_set.length( );
	   result.append( character_set.at( index ) );
	}

	return result;
}

bool is_elevated( )
{
	HANDLE hToken = nullptr;

	if ( !OpenProcessToken( GetCurrentProcess( ), TOKEN_QUERY, &hToken ) )
		return FALSE;

	TOKEN_ELEVATION token;
	DWORD cbSize = sizeof( TOKEN_ELEVATION );

	if ( GetTokenInformation( hToken, TokenElevation, &token, sizeof( token ), &cbSize ) )
	{
		CloseHandle( hToken );
		return token.TokenIsElevated;
	}

	return false;
}

int query_fastboot( )
{
    QSettings settings( xorstr_( "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\SessionManager\\Power" ), QSettings::NativeFormat );
    return settings.value( xorstr_( "HiberbootEnabled " ) ).toInt( );
}

int get_windows_build( )
{
	QSettings settings( xorstr_( "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion" ), QSettings::NativeFormat );
	return settings.value( xorstr_( "ReleaseId" ) ).toInt( );
}

DWORD get_process_id( LPCWSTR exe_name )
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

	if ( hSnapShot == INVALID_HANDLE_VALUE )
		return NULL;

	PROCESSENTRY32 pe = { 0 };

	pe.dwSize = sizeof( PROCESSENTRY32 );

	for ( BOOL	success  = Process32First( hSnapShot, &pe );
				success == TRUE;
				success  = Process32Next( hSnapShot, &pe ) )
	{
		if ( wcscmp( exe_name, pe.szExeFile ) == 0 )
		{
			CloseHandle( hSnapShot );
			return pe.th32ProcessID;
		}
	}

	CloseHandle( hSnapShot );
	return NULL;
}

QString get_key_name( unsigned int virtual_key )
{
	if ( virtual_key == VK_LBUTTON )
		return "Left Mouse";

	if ( virtual_key == VK_RBUTTON )
		return "Right Mouse";

	if ( virtual_key == VK_MBUTTON )
		return "Middle Mouse";

	char buffer[1024] = { 0 };

	long scan_code = MapVirtualKeyA( virtual_key, MAPVK_VK_TO_VSC );
	long param = ( scan_code << 16 );

	int result = GetKeyNameTextA( param, buffer, 1024 );

	if ( !result )
		return QString::number( virtual_key );

	return QString::fromStdString( buffer );
}

bool delete_usn_journal( const std::string& volume )
{
	//UNREFERENCED_PARAMETER( volume );
	//return false;

	if ( volume.empty( ) )
	{
		qDebug( ) << "Invalid volume";
		return false;
	}

	HANDLE hVolume = CreateFileA( volume.c_str( ), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL );

	if ( hVolume == INVALID_HANDLE_VALUE )
	{
		qDebug( ) << "Invalid handle";
		return false;
	}

	DWORD dwBuffer = 0;
	USN_JOURNAL_DATA USNJournalData;

	if ( !DeviceIoControl( hVolume, FSCTL_QUERY_USN_JOURNAL, NULL, 0, &USNJournalData, sizeof( USN_JOURNAL_DATA ), &dwBuffer, NULL ) )
	{
		qDebug( ) << "DeviceIoControl failed";
		CloseHandle( hVolume );
		return false;
	}

	qDebug( ) << "USNJournal" << USNJournalData.UsnJournalID;

	DELETE_USN_JOURNAL_DATA USNJournalDataDelete;
	USNJournalDataDelete.DeleteFlags	= USN_DELETE_FLAG_DELETE;
	USNJournalDataDelete.UsnJournalID	= USNJournalData.UsnJournalID;

	if ( !DeviceIoControl( hVolume, FSCTL_DELETE_USN_JOURNAL, &USNJournalDataDelete, sizeof( DELETE_USN_JOURNAL_DATA ), NULL, 0, &dwBuffer, NULL ) )
	{
		qDebug( ) << "DeviceIoControl failed" << GetLastError( );
		CloseHandle( hVolume );
		return false;
	}

	CloseHandle( hVolume );

	qDebug( ) << "[+] USN journal wiped\n";

	return true;

}

bool clean_other_caches( )
{
	system( "FSUTIL USN DELETEJOURNAL /D C:" );
	//system( "RunDll32.exe InetCpl.cpl,ClearMyTracksByProcess 255" );
	system( "ipconfig /flushdns" );

	return true;
}

bool set_debug_privilege( )
{
    TOKEN_PRIVILEGES NewState;
    LUID luid;
    HANDLE hToken;

    OpenProcessToken( GetCurrentProcess( ), TOKEN_ADJUST_PRIVILEGES, &hToken );
    LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &luid );

    NewState.PrivilegeCount = 1;
    NewState.Privileges[0].Luid = luid;
    NewState.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if ( !AdjustTokenPrivileges( hToken, FALSE, &NewState, sizeof( NewState ), NULL, NULL ) )
    {
        printf( "[!] failed to set privilege, GetLastError( )=%lx\n", GetLastError( ) );
        CloseHandle( hToken );
        return false;
    }

    CloseHandle( hToken );
    return true;
}

