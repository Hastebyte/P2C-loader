#include "web_api.h"

#include <QDebug>
#include <QCoreApplication>

namespace util
{

    size_t curl_callback_write_file( void *ptr, size_t size, size_t nmemb, FILE* stream )
    {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    /*
    static size_t curl_callback_memory_binary( void* contents, size_t size, size_t nmemb, void* userp )
    {
        qDebug( ) << "curl_callback_memory_binary";

        size_t real_size = size * nmemb;
        struct memory_struct* mem = reinterpret_cast<struct memory_struct*>( userp );

        if ( real_size > max_file_size )
        {
            qDebug( ) << xorstr_( "[!] max file size exceeded" );
            return 0;
        }

        memcpy( &mem->memory[mem->size], contents, real_size );
        mem->size += real_size;
        mem->memory[mem->size] = 0;

        return real_size;
    }
    */

    static size_t curl_callback_memory_binary( void* contents, size_t size, size_t nmemb, void* userp )
    {
        //qDebug( ) << "curl_callback_memory_binary";


      size_t realsize = size * nmemb;
      struct memory_struct *mem = ( struct memory_struct * )userp;

      char *ptr = reinterpret_cast<char*>( realloc( mem->memory, mem->size + realsize + 1 ) );

      if( ptr == nullptr )
      {
        /* out of memory! */
        //printf("not enough memory (realloc returned NULL)\n");
        return 0;
      }

      mem->memory = ptr;
      memcpy(&(mem->memory[mem->size]), contents, realsize);
      mem->size += realsize;
      mem->memory[mem->size] = 0;

      return realsize;
    }

    int progress_func( void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded )
    {
        //qDebug( ) << "TotalToDownload:" << TotalToDownload << "NowDownloaded" << NowDownloaded;

        QCoreApplication::processEvents( );
        return 0;
    }

    bool curl_http_get_binary( const QString& url, memory_struct& buffer )
    {
        buffer.memory = reinterpret_cast<char*>( malloc( 1 ) );
        buffer.size = 0;    /* no data at this point */


        CURL* curl_handle = curl_easy_init( );
        curl_easy_setopt( curl_handle, CURLOPT_URL, url.toStdString( ).c_str( ) );
        curl_easy_setopt( curl_handle, CURLOPT_WRITEFUNCTION, curl_callback_memory_binary );
        curl_easy_setopt( curl_handle, CURLOPT_WRITEDATA, reinterpret_cast<void *>( &buffer ) );
        curl_easy_setopt( curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0" );

        curl_easy_setopt( curl_handle, CURLOPT_NOPROGRESS, FALSE );
        curl_easy_setopt( curl_handle, CURLOPT_PROGRESSFUNCTION, progress_func );

        /* get it! */
        CURLcode res = curl_easy_perform( curl_handle );

        /* check for errors */
        if( res != CURLE_OK )
        {
          //qDebug( ) << "[!] curl failed\n";
          //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
          //printf("%lu bytes retrieved\n", (unsigned long)buffer.size);
          //qDebug( ) << "[-] curl get binary good" << buffer.size;
        }

        /* cleanup curl stuff */
        curl_easy_cleanup( curl_handle );

        //free(chunk.memory);

        return true;
    }

    const QString curl_http_get( const QString& address )
    {
        CURL* curl = curl_easy_init( );

        if ( !curl )
        {
            printf( "[!] failed to init curl\n" );
            //curl_global_cleanup( );

            return QString( );
        }

        std::string buffer;

        curl_easy_setopt( curl, CURLOPT_URL, address.toStdString( ).c_str( ) );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0L );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 0L );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_callback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &buffer );
        //curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );

        CURLcode result = curl_easy_perform( curl );

        if ( result != CURLE_OK )
        {
            printf( "[!] curl_easy_perform() failed: %s\n", curl_easy_strerror( result ) );
            curl_easy_cleanup( curl );
            return QString( );
        }

        curl_easy_cleanup( curl );
        //curl_global_cleanup( );

        return QString::fromStdString( buffer );
    }

    const QString curl_http_post( const QString& address, const char* data )
    {
        //curl_global_init( CURL_GLOBAL_DEFAULT );

        //qDebug( ) << "[-] http_post got data:" << data;

        CURL* curl = curl_easy_init( );

        if ( !curl )
        {
            //printf( "[!] failed to init curl\n" );
            //curl_global_cleanup( );

            return QString( );
        }

        std::string buffer;

        curl_easy_setopt( curl, CURLOPT_URL, address.toStdString( ).c_str( ) );
        curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data );
        //curl_easy_setopt( curl, CURLOPT_POSTFIELDSIZE, strlen( data ) );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0L );
        curl_easy_setopt( curl, CURLOPT_SSL_VERIFYHOST, 0L );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, curl_callback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, &buffer );
        //curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );

        CURLcode result = curl_easy_perform( curl );

        if ( result != CURLE_OK )
        {
            //printf( "[!] curl_easy_perform() failed: %s\n", curl_easy_strerror( result ) );
            curl_easy_cleanup( curl );
            return QString( );
        }

        curl_easy_cleanup( curl );
        //curl_global_cleanup( );

        return QString::fromStdString( buffer );
    }

    size_t curl_callback( void* contents, size_t size, size_t nmemb, std::string* user_data )
    {
        size_t adjusted_length = size * nmemb;

        //qDebug( ) << " curl_callback" << nmemb;

        try
        {
            user_data->append( reinterpret_cast<const char*>( contents ), adjusted_length );
        } catch ( std::bad_alloc& e )
        {
            UNREFERENCED_PARAMETER( e );
            return 0;
        }

        return adjusted_length;
    }


	bool post_url( const std::string url, const std::string& resource, const std::string& data, std::string& return_data, unsigned long timeout )
	{
        return false;

		UNREFERENCED_PARAMETER( timeout );

		//printf( "[+] POST URL: %s Resource %s\n", url.c_str( ), resource.c_str( ) );

		HINTERNET hOpen = InternetOpenA( USER_AGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );

		if ( !hOpen )
		{
			//printf( "[!] Failed to call InternetOpen( ), GetLastError( )=%d\n", GetLastError( ) );
			InternetCloseHandle( hOpen );

			return false;
		}

		//BOOL b = InternetSetOption( hOpen, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof( timeout ) );

		////printf( "---> set timeout: %d\n", b );

		HINTERNET hConnect = InternetConnectA( hOpen, url.c_str( ), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, WININET_API_FLAG_SYNC, 0 );

		if ( !hConnect )
		{
			printf( "[!] Failed to call InternetConnect( ), GetLastError( )=%d\n", GetLastError( ) );

			InternetCloseHandle( hConnect );
			InternetCloseHandle( hOpen );
			return false;
		}

		// disable SSL check

		DWORD dwFlags;
		DWORD dwBuffLen = sizeof( dwFlags );

		if ( InternetQueryOption( hConnect, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, &dwBuffLen ) )
		{
			//dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;

			dwFlags |=	SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
						SECURITY_FLAG_IGNORE_CERT_DATE_INVALID |
						SECURITY_FLAG_IGNORE_UNKNOWN_CA;

			InternetSetOption( hConnect, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof( dwFlags ) );
		}

		// qDebug( ) << "[+] Open Request";

		static char lpszHeader[] = "Content-Type: application/x-www-form-urlencoded\r\n";
		LPCSTR lplpszTypes[2] = { "*/*", NULL };

		HINTERNET hRequest = HttpOpenRequestA( hConnect, "POST", resource.c_str( ), NULL, NULL, lplpszTypes, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0 );

		if ( !hRequest )
		{
			printf( "[!] Failed to call HttpOpenRequest( ), GetLastError( )=%d\n", GetLastError( ) );

			InternetCloseHandle( hRequest );
			InternetCloseHandle( hConnect );
			InternetCloseHandle( hOpen );
			return false;
		}

		// disable SSL check 2

		//if ( InternetQueryOption( hRequest, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, &dwBuffLen ) )
		//{
		//	dwFlags |= SECURITY_FLAG_IGNORE_UNKNOWN_CA;
		//	InternetSetOption( hConnect, INTERNET_OPTION_SECURITY_FLAGS, &dwFlags, sizeof( dwFlags ) );
		//}

		// qDebug( ) << "[+] Send Request";

		size_t hDataLength = data.length( );

		if ( !HttpSendRequestA( hRequest, lpszHeader, ( DWORD )-1, const_cast<PCHAR>( data.c_str( ) ), ( DWORD )hDataLength ) )
		{
			printf( "[!] Failed to call HttpSendRequest( ), GetLastError( )=%d\n", GetLastError( ) );

			InternetCloseHandle( hConnect );
			InternetCloseHandle( hOpen );
			return false;
		}

		// qDebug( ) << "[+[ Read Response";

		// read response

		const DWORD blocksize = 4096;
		DWORD received = 0;
		std::string temp;
		std::string block( blocksize, 0 );

		while ( InternetReadFile( hRequest, &block[0], blocksize, &received ) && received )
		{
			block.resize( received );
			temp += block;

			// qDebug( ) << "[+] recvd:" << received;
		}

		return_data = temp;

		InternetCloseHandle( hRequest );
		InternetCloseHandle( hConnect );
		InternetCloseHandle( hOpen );

		return true;
	}

	bool get_url( const std::string& url, std::string& data )
	{
        return false;

		// INTERNET_OPEN_TYPE_DIRECT
		// INTERNET_OPEN_TYPE_PRECONFIG

		HINTERNET hOpen = InternetOpenA( USER_AGENT, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0 );

		if ( !hOpen )
		{
			//printf( "[!] Failed to call InternetOpen( ), GetLastError( )=%d\n", GetLastError( ) );

			InternetCloseHandle( hOpen );
			return false;
		}

		DWORD dwRequestFlags = INTERNET_FLAG_NO_UI | INTERNET_FLAG_NO_AUTH | INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_NO_CACHE_WRITE;

		HINTERNET hFile = InternetOpenUrlA( hOpen, url.c_str( ), NULL, 0, dwRequestFlags, 0 );

		if ( !hFile )
		{
			//printf( "[!] Failed to call InternetOpenUrl( ), GetLastError( )=%d\n", GetLastError( ) );

			InternetCloseHandle( hFile );
			InternetCloseHandle( hOpen );
			return false;
		}

		//PCHAR pBuffer = reinterpret_cast<PCHAR>( malloc( buffersize ) );
		//SecureZeroMemory( pBuffer, buffersize );

		//DWORD dwRead;

		//if ( InternetReadFile( hFile, pBuffer, buffersize, &dwRead ) )
		//	data += pBuffer;

		// free( pBuffer );

		const DWORD blocksize = 4096;
		DWORD received = 0;
		std::string temp;
		std::string block( blocksize, 0 );

		while ( InternetReadFile( hFile, &block[0], blocksize, &received ) && received )
		{
			block.resize( received );
			temp += block;
		}

		data = temp;


		InternetCloseHandle( hFile );
		InternetCloseHandle( hOpen );

		return true;
	}

	bool valid_ip( const std::string& ip )
	{
		struct sockaddr_in sa;
		int result = inet_pton( AF_INET, ip.c_str( ), &( sa.sin_addr ) );
		return result != 0;
	}

	void test_tcp_table( )
	{
		// GET NECESSARY SIZE OF TCP TABLE
		DWORD table_size = 0;
		GetExtendedTcpTable( 0, &table_size, false, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0 );

		// ALLOCATE BUFFER OF PROPER SIZE FOR TCP TABLE
		MIB_TCPTABLE_OWNER_MODULE* allocated_ip_table = ( MIB_TCPTABLE_OWNER_MODULE* )malloc( table_size );

		//printf( "[+] allocated ip table: %p\n", allocated_ip_table );

		DWORD result = GetExtendedTcpTable( allocated_ip_table, &table_size, false, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0 );

		if ( result != NO_ERROR )
		{
			//printf( "[!] failed to enumerate\n" );
			free( allocated_ip_table );
			return;
		}

		for ( unsigned int entry_index = 0; entry_index < allocated_ip_table->dwNumEntries; ++entry_index )
		{
			unsigned int int_ip = allocated_ip_table->table[entry_index].dwRemoteAddr;

			if ( !int_ip )
				continue;

			char str_ip[INET_ADDRSTRLEN+1];
			inet_ntop( AF_INET, &int_ip, str_ip, INET_ADDRSTRLEN );

			printf( "    remoteaddr = %lx {%s}\n", allocated_ip_table->table[entry_index].dwRemoteAddr, str_ip );

		}

		free( allocated_ip_table );
	}
}
