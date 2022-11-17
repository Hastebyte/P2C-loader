#include "hwid.h"

QString hwid_cpu_info( )
{
      DWORD length = 0;
      GetLogicalProcessorInformation( nullptr, &length );

      PSYSTEM_LOGICAL_PROCESSOR_INFORMATION cpu_info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>( malloc( length ) );

      if ( !GetLogicalProcessorInformation( cpu_info, &length ) )
      {
     //qDebug( ) << "[!] Failed to call GetLastError:" << GetLastError( );
	 return "";
      }

      unsigned int l1_cache_count = 0;
      unsigned int l2_cache_count = 0;
      unsigned int l3_cache_count = 0;
      unsigned int core_count = 0;

      for ( int i = 0; i != length / sizeof( SYSTEM_LOGICAL_PROCESSOR_INFORMATION ); ++i )
      {
	 if (cpu_info[i].Relationship == RelationCache && cpu_info[i].Cache.Level == 1 )
	    l1_cache_count++;

	 if (cpu_info[i].Relationship == RelationCache && cpu_info[i].Cache.Level == 2 )
	    l2_cache_count++;

	 if (cpu_info[i].Relationship == RelationCache && cpu_info[i].Cache.Level == 3 )
	    l3_cache_count++;

	 if ( cpu_info[i].Relationship == RelationProcessorCore )
	    core_count++;
      }

      //qDebug( ) << "L1 cache size:" << l1_cache_count;
      //qDebug( ) << "L2 cache size:" << l2_cache_count;
      //qDebug( ) << "L3 cache size:" << l3_cache_count;
      //qDebug( ) << "Core Count:" << core_count;

      free( cpu_info );

      QString result =
	    QString::number( l1_cache_count ) + "|" +
	    QString::number( l2_cache_count ) + "|" +
	    QString::number( l3_cache_count ) + "|" +
	    QString::number( core_count );

      return result;
}

QString hwid_cpu_name( )
{
	int CPUInfo[4] = { -1 };
	char CPUBrandString[128] = {0};
	__cpuid( CPUInfo, 0x80000000 );
	unsigned int nExIds = CPUInfo[0];

	memset( CPUBrandString, 0, sizeof( CPUBrandString ) );

	// Get the information associated with each extended ID.
	for ( int i = 0x80000000; i <= nExIds; ++i )
	{
	__cpuid( CPUInfo, i );

	// Interpret CPU brand string.
	if ( i == 0x80000002 )
	    memcpy( CPUBrandString, CPUInfo, sizeof( CPUInfo ) );
	else if ( i == 0x80000003 )
	    memcpy( CPUBrandString + 16, CPUInfo, sizeof( CPUInfo ) );
	else if ( i == 0x80000004 )
	    memcpy( CPUBrandString + 32, CPUInfo, sizeof( CPUInfo ) );
	}

	return CPUBrandString;
}

QString get_permanent_address( )
{
	ULONG buffer_size = 0;
	DWORD result = GetAdaptersInfo( nullptr, &buffer_size );

	if ( result != ERROR_BUFFER_OVERFLOW )
	{
		// printf( "[!] failed to called GetAdaptersInfo( NULL, size ), error=%lx\n", result );
		return QString( );
	}

	PIP_ADAPTER_INFO adapter_info = reinterpret_cast<PIP_ADAPTER_INFO>( malloc( buffer_size ) );
	result = GetAdaptersInfo( adapter_info, &buffer_size );

	if ( ERROR_SUCCESS != result )
	{
		// printf( "[!] failed to called GetAdaptersInfo( buffer, size ), error=%lx\n", result );
		free( adapter_info );
		return QString( );
	}

	QString mac_string;

	if ( !oid_address_request( adapter_info->AdapterName, mac_string ) )
		mac_string = "000000000000";

	free( adapter_info );
	return mac_string;
}

bool oid_address_request( const char* service_name, QString& mac_string )
{
	QString service;
	service.sprintf( "\\\\.\\%s", service_name );

	HANDLE device = CreateFile( service.toStdWString( ).c_str( ), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr );

	if ( device == INVALID_HANDLE_VALUE )
	{
		qDebug( ) << "Failed to open device:" << service;
		return false;
	}

	const DWORD query_global_stats		= 0x170002;
	const DWORD oid_permanent_address	= 0x01010101;

	unsigned long bytes_returned;
	unsigned char result[256] = {0};

	if ( !DeviceIoControl( device, query_global_stats, ( LPVOID )( &oid_permanent_address ), 4, result, 256, &bytes_returned, nullptr ) )
	{
		CloseHandle( device );
		return false;
	}

	wchar_t str_buffer[512] = {0};
	wsprintfW( str_buffer, L"\\\\.\\%s", service_name );

	swprintf_s( str_buffer, sizeof( str_buffer ), L"%02X%02X%02X%02X%02X%02X",
		result[0],
		result[1],
		result[2],
		result[3],
		result[4],
		result[5] );

	mac_string = QString::fromWCharArray( str_buffer );
	CloseHandle( device );
	return true;
}
