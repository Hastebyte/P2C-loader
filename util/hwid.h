#ifndef HARDWAREID
#define HARDWAREID

#include <windows.h>
#include <stdio.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <string>

#include <Sysinfoapi.h>

#include <QString>
#include <QDebug>

#pragma comment( lib, "user32.lib" )
#pragma comment( lib, "iphlpapi.lib" )



QString hwid_cpu_info( );
QString hwid_cpu_name( );

QString get_permanent_address( );
//bool get_permanent_address( std::wstring& mac_string );
bool oid_address_request( const char* service_name, QString& mac_string );

#endif
