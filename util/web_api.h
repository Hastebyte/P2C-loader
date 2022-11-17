#ifndef WEB_API_H
#define WEB_API_H

#include <ws2tcpip.h>
#include <wininet.h>

#include <shlobj.h>

#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <time.h>

#pragma comment( lib, "wininet.lib" )
#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "urlmon.lib" )
#pragma comment( lib, "iphlpapi.lib" )

#define USER_AGENT "Mozilla/5.0 Firefox/3.0.1"

#include <iphlpapi.h>
#include <winsock2.h>

#include "process.h"
#include "shared.h"

#include "curl/curl.h"

namespace util
{
    struct memory_struct
    {
        char* memory;
        size_t size;
    };

    size_t curl_callback_write_file( void *ptr, size_t size, size_t nmemb, FILE* stream );
    bool curl_http_get_binary( const QString& url, memory_struct& buffer );

    const QString curl_http_get( const QString& address );
    const QString curl_http_post( const QString& address, const char* data );

    size_t curl_callback( void* contents, size_t size, size_t nmemb, std::string* user_data );


    //bool post_url( const std::string url, const std::string& resource, const std::string& data, std::string& return_data, unsigned long timeout = 2000 );
    //bool get_url( const std::string& url, std::string& data );
	bool valid_ip( const std::string& ip );

	// battleye anti-table dump
	// iphlpapi.GetExtendedTcpTable -> iphlpapi.NsiAllocateAndGetTable -> eventually: netio.sys NsiEnumerateObjectsAllParametersEx

	void test_tcp_table( );
}

#endif
