#include "authenticator.h"

// https://glot.io/snippets/fgyt7mt18q/raw/message - server message
// https://glot.io/snippets/fgyt61grqw/raw/connector - auth server
// https://glot.io/snippets/fgyt6vz8pm/raw/hash - hash list

__declspec( noinline ) bool authenticator::validate_key_format( )
{
	VMProtectBeginMutation( "validate_key_format" );
	////VIRTUALIZER_MUTATE_ONLY_START

	if ( m_serial_key.length( ) != 40 )
		return false;

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return true;

}

__declspec( noinline ) QString authenticator::pick_key_cache_server( )
{
    //VMProtectBeginMutation( "pick_key_cache_server" );
	//////VIRTUALIZER_MUTATE_ONLY_START

    return xorstr_( "https://glot.io/snippets/xxx/raw/hash" );
    //return xorstr_( "http://pastebin.com/raw/xxx" );

    //VMProtectEnd( );
	//////VIRTUALIZER_MUTATE_ONLY_END
}

__declspec( noinline ) QString authenticator::pick_ip_cache_server( )
{
	//////VIRTUALIZER_MUTATE_ONLY_START
    //VMProtectBeginMutation( "pick_ip_cache_server" );

    return xorstr_( "https://glot.io/snippets/xxx/raw/connector" );
    //return xorstr_( "http://pastebin.com/raw/xxx" );

    //VMProtectEnd( );
	//////VIRTUALIZER_MUTATE_ONLY_END
}

__declspec( noinline ) QByteArray authenticator::get_url_native( const QString& url )
{
	VMProtectBeginMutation( "get_url_native" );
	////VIRTUALIZER_MUTATE_ONLY_START

    //std::string data;

    //if ( !util::get_url( url.toStdString( ), data ) )
    //{
    //	return QByteArray( );
    //}

	// QMessageBox::information( nullptr, "debug1", "debug length:" + QString::number( data.length( ) ) );

    //VMProtectEnd( );

    //QByteArray byte_array( data.c_str( ), ( int )data.length( ) );

	////VIRTUALIZER_MUTATE_ONLY_END
    return QByteArray( );
}

__declspec( noinline ) QByteArray authenticator::post_url_native( const QString& url, const QString& resource, const QString& data )
{
	VMProtectBeginMutation( "post_url_native" );
	////VIRTUALIZER_MUTATE_ONLY_START

    //std::string return_data;

    //if ( !util::post_url( url.toStdString( ), resource.toStdString( ), data.toStdString( ), return_data ) )
    //{
    //	return QByteArray( );
    //}

	// QMessageBox::information( nullptr, "debug1", "debug length:" + QString::number( data.length( ) ) );

    //VMProtectEnd( );

	////VIRTUALIZER_MUTATE_ONLY_END
    return QByteArray( );
}

__declspec( noinline ) QByteArray authenticator::get_url( const QString& url )
{
	VMProtectBeginMutation( "get_url" );
	////VIRTUALIZER_MUTATE_ONLY_START

	QNetworkAccessManager manager;


	QNetworkReply* response = manager.get( QNetworkRequest( QUrl( url ) ) );
	response->setParent( nullptr );

	////QSslConfiguration sslConf = response->sslConfiguration( );
	////sslConf.setPeerVerifyMode( QSslSocket::VerifyNone );
	////sslConf.setProtocol( QSsl::SslV2 );
	////response->setSslConfiguration( sslConf );
	////response->ignoreSslErrors( );

	QEventLoop event;
	connect( response, SIGNAL( finished( ) ), &event, SLOT( quit( ) ) );
	event.exec( );

	QByteArray data_string = response->readAll( );
	//QMessageBox::information( nullptr, "debug1", "debug length:" + QString::number( data.length( ) ) );

    manager.clearAccessCache( );

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return data_string;
}

__declspec( noinline ) QByteArray authenticator::post_url( const QString& url, const QString& post_data )
{
	VMProtectBeginMutation( "post_url" );
	////VIRTUALIZER_MUTATE_ONLY_START

	QNetworkAccessManager manager;
	QNetworkRequest request( url );

	request.setHeader( QNetworkRequest::ContentTypeHeader, xorstr_( "application/x-www-form-urlencoded" ) );

	QNetworkReply* response = manager.post( request, post_data.toUtf8( ) );
	QEventLoop event;
	connect( response, SIGNAL( finished( ) ), &event, SLOT( quit( ) ) );
	event.exec( );

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return response->readAll( );
}

__declspec( noinline ) bool authenticator::validate_ip( const QString& address )
{
	VMProtectBeginMutation( "validate_ip" );
	////VIRTUALIZER_MUTATE_ONLY_START

	QHostAddress host_address( address );

	if ( host_address.isNull( ) )
		return false;

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return true;

}

__declspec( noinline ) QString authenticator::get_auth_server( )
{
	VMProtectBeginMutation( "get_auth_server" );
	////VIRTUALIZER_MUTATE_ONLY_START

    QString encrypted_address = util::curl_http_get( pick_ip_cache_server( ) );

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return decode_decrypt( encrypted_address ).trimmed( );

}



__declspec( noinline ) QString authenticator::get_monitor_serial( )
{
	return "";
}

__declspec( noinline ) bool authenticator::validate_key_against_cache( )
{
	VMProtectBeginMutation( "validate_key_against_cache" );
	////VIRTUALIZER_MUTATE_ONLY_START

	QString cache_url = pick_key_cache_server( );

	if ( cache_url.length( ) == 0 )
	{
		////qDebug( ) << "[+] bad key cache url";
		return false;
	}

    QString cached_serials = util::curl_http_get( cache_url );

	if ( cached_serials.length( ) == 0 )
	{
		////qDebug( ) << "[+] no cached serials returned";
        QMessageBox::information( nullptr, xorstr_( "Network Issue" ), xorstr_( "Unable to contact cache server. GLE:" ) + GetLastError( ) );
		return false;
	}

    // Create a hash of the partial key for check

	QString partial_key = m_serial_key.left( 20 );

	// //qDebug( ) << "[+] Partial key:" << partial_key;

	QByteArray hash_array = QCryptographicHash::hash( partial_key.toUtf8( ), QCryptographicHash::Sha256 ).toHex( );
	QString hash_string = QString( hash_array );

	//qDebug( ) << "[+] Hash:" << hash_string;

	// QMessageBox::information( nullptr, "stat", "okay 1" );


	QStringList hashed_key_list = cached_serials.split( QRegExp( xorstr_( "[\r\n]" ) ), QString::SkipEmptyParts );

	for ( const auto& hashed_key : hashed_key_list )
	{
		if ( hashed_key == hash_string )
		{
			////qDebug( ) << "[+] found";
			return true;
		}
	}

	////qDebug( ) << "[+] key not found";

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return false;

}

__declspec( noinline ) unsigned int authenticator::validate_key( )
{
	VMProtectBeginMutation( "validate_key" );
	////VIRTUALIZER_MUTATE_ONLY_START

    //QString ip = get_auth_server( );
	//qDebug( ) << "[+] using server:" << ip;

    //if( ip.isEmpty( ) )
    //{
    //	return ui_failure_invalid_load_balancer;
    //}

    QString url = xorstr_( "http://127.0.0.1/index.php" ); // xorstr_( "http://" );




	QString hw_stamp = hwid_cpu_name( ) + "|" + hwid_cpu_info( ); //get_permanent_address( );

	QByteArray hw_stamp_byte_array;
	hw_stamp_byte_array.append( hw_stamp );
	hw_stamp = hw_stamp_byte_array.toBase64( );


	QDateTime current( QDateTime::currentDateTime( ) );
    unsigned int unix_time = current.toTime_t( );

    std::string payload;
    payload += m_serial_key.toStdString( );
    payload += "|";
    payload += hw_stamp.toStdString( );
    payload += "|";
    payload += std::to_string( m_game_id );
    payload += "|";
    payload += std::to_string( unix_time );

    //qDebug( ) << "[-] payload:" << payload.c_str( );

    // encrypt and base64 encode

    encrypt_forward_simple( payload, payload );
    //encrypt_forward_simple( payload, payload );
    //qDebug( ) << "[-] payload after:" << payload.c_str( );

    payload = util::base64_encode( payload );

    std::string final_payload = xorstr_( "bin=" );
    final_payload += payload;
    final_payload += xorstr_( "&a=1" );

    // use curl to encode the string

    //CURL* curl = curl_easy_init( );
    //char* payload_escaped = curl_easy_escape( curl, payload.c_str( ), payload.length( ) );
    //curl_easy_cleanup( curl );

    // combine this

    //qDebug( ) << "[-] escaped and encoded:" << final_payload.c_str( );

    //QString post_payload = "bin=";
    //post_payload += QString::fromStdString( payload );

    // submit request

    QString response = util::curl_http_post( url, final_payload.c_str( ) );

	if ( response.length( ) == 0 )
	{
		return ui_failure_invalid_server_response;
	}

	// decrypt

	response = decode_decrypt( response );

    //qDebug( ) << "[+] response:" << response;

	QStringList parts = response.split( "|" );

	if ( parts.size( ) < 3 )
	{
		return ui_failure_invalid_server_response2;
	}

	// check loader version here

	////VIRTUALIZER_MUTATE_ONLY_START

	bool ok;

	unsigned int server_version = parts.at( 0 ).toUInt( &ok, 10 );
	unsigned int server_code    = parts.at( 3 ).toUInt( &ok, 10 );

	if ( loader_version < server_version )
	{
		return ui_failure_invalid_loader_version;
	}

	unsigned int ui_return_code = ui_failure_invalid_server_response3;

	////VIRTUALIZER_MUTATE_ONLY_END

	switch ( server_code )
	{
		case server_success:
		case server_success_first_registration:
			m_cheat_url  = parts.at( 4 );
			m_driver_url = parts.at( 6 );
			ui_return_code = ui_success;
			break;
		case server_failure_invalid_serial:
			ui_return_code = ui_failure_invalid_serial;
			break;
		case server_failure_invalid_computerid:
			ui_return_code = ui_failure_invalid_computerid;
			break;
		case server_failure_expired_serial:
			ui_return_code = ui_failure_expired_serial;
			break;
		case server_failure_maintenance:
			ui_return_code = ui_failure_maintenance;
			break;
		default:
			break;
	}





	VMProtectEnd( );

	return ui_return_code;


}

__declspec( noinline ) bool authenticator::clean_old_files( const QString& path )
{
	VMProtectBeginMutation( "clean_old_files" );

    qDebug( ) << "[-] clean";

	QDirIterator it( path, QStringList( ) << xorstr_( "*.exe" ) << xorstr_( "*.dll" ), QDir::Files );

	while ( it.hasNext( ) )
	{
		QFileInfo file_info( it.next( ) );

		if ( file_info.fileName( ).length( ) == 26 )
		{
			// //qDebug( ) << file_info.fileName( );

			QFile file( file_info.filePath( ) );
			file.remove( );
		}
	}

	VMProtectEnd( );
	return true;
}

__declspec( noinline ) unsigned int authenticator::download_files( )
{
	VMProtectBeginMutation( "download_files" );
	////VIRTUALIZER_MUTATE_ONLY_START

	if ( m_driver_url.length( ) == 0 )
		return false;

    //wchar_t temp_path_buffer[MAX_PATH] = { 0 };

   // if ( !SHGetSpecialFolderPath( nullptr, temp_path_buffer, CSIDL_COMMON_APPDATA, FALSE ) )
    //{
    //	return ui_failure_program_path;
    //}

	// create folder + clean old files

	// C:\Program Files\Microsoft SQL Server\90\Shared


    QString destination_folder = QCoreApplication::applicationDirPath( ) + xorstr_( "/local/" );

    //if ( m_game_id == hardware_spoof || m_game_id == hardware_spoof2 )
    //{
    //    destination_folder = QStandardPaths::writableLocation( QStandardPaths::TempLocation );
    //    destination_folder + "\\";
    //}

   // QMessageBox::information( nullptr, "Screenshot message", destination_folder );

	QDir directory( destination_folder );

	if ( !directory.exists( ) )
	{
		directory.mkpath( destination_folder );
	} else {
		clean_old_files( destination_folder );
	}

	// download cheat to folder

   // qDebug( ) << "[-] download path:" << destination_folder;
   // qDebug( ) << "[-] downloading:" << m_cheat_url;

    //QString data = util::curl_http_get( m_cheat_url );

    util::memory_struct user_binary;
    curl_http_get_binary( m_cheat_url, user_binary );


	//qDebug( ) << "[-] length:" << cheat_byte_array.length( );

    if ( user_binary.size == 0 )
	{
		return ui_failure_download_cheat;
	}

    // if dll, save bytes for manual mapping -- if exe then write out to disk

	if ( m_cheat_url.contains( ".dll" ) )
    {
        m_cheat_file = destination_folder + random_string( 22 ) + xorstr_( ".dll" );
        m_cheat_byte_array = QByteArray::fromRawData( user_binary.memory, ( int )user_binary.size );

        if ( m_game_id == game_overwatch )
        {
            //qDebug( ) << "[-] special case: overwatch" ;

            QFile file( m_cheat_file );
            file.open( QIODevice::WriteOnly );
            qint64 bytes_written = file.write( user_binary.memory, static_cast<int64_t>( user_binary.size ) );
            file.close( );

        }


    } else {
		m_cheat_file = destination_folder + random_string( 22 ) + xorstr_( ".exe" );


        QFile file( m_cheat_file );
        file.open( QIODevice::WriteOnly );
        qint64 bytes_written = file.write( user_binary.memory, static_cast<int64_t>( user_binary.size ) );
        file.close( );

       // QMessageBox::information( nullptr, "Screenshot message", QString::number( bytes_written ) );

        if ( bytes_written == -1 )
        {
            return ui_failure_failed_to_write_file;
        }
    }


	//qDebug( ) << "[-] downloaded to:" << m_cheat_file;




	// download driver to array

    //qDebug( ) << "[+] Download driver:" << m_driver_url;

    util::memory_struct kernel_binary;
    curl_http_get_binary( m_driver_url, kernel_binary );

    //qDebug( ) << "[+] downloaded";

    m_driver_byte_array = QByteArray::fromRawData( kernel_binary.memory, ( int )kernel_binary.size );
	m_decrypted_driver_byte_array = decrypt( m_driver_byte_array );
	m_driver_byte_array.clear( );


    //qDebug( ) << "[+] decrypted:" << m_decrypted_driver_byte_array.at( 0 ) << m_decrypted_driver_byte_array.at( 1 );


	if ( m_decrypted_driver_byte_array.length( ) == 0 )
	{
		return ui_failure_download_driver;
	}

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return ui_success;

}

__declspec( noinline ) QByteArray authenticator::driver_byte_array( )
{
	return this->m_decrypted_driver_byte_array;
}

__declspec( noinline ) QString authenticator::cheat_file( )
{
	return this->m_cheat_file;
}
