#include "MainWindow.h"
#include "ui_MainWindow.h"

//#define LOADER_CHINA_BUILD 0

MainWindow::MainWindow( QWidget* parent ) : QMainWindow( parent ), ui( new Ui::MainWindow )
{
	VMProtectBeginMutation( "MainWindow::MainWindow" );
	////VIRTUALIZER_TIGER_WHITE_START

	ui->setupUi( this );

	setWindowTitle( " " );

	m_block_auth = false;
    curl_global_init( CURL_GLOBAL_DEFAULT );

    ui->cmbGame->addItem( xorstr_( "R6 Siege" ), QVariant( game_siege2 ) );
    ui->cmbGame->addItem( xorstr_( "DayZ" ), QVariant( game_dayz ) );
    ui->cmbGame->addItem( xorstr_( "Rust" ), QVariant( game_rust2 ) );
    ui->cmbGame->addItem( xorstr_( "Scum" ), QVariant( game_scum ) );
    ui->cmbGame->addItem( xorstr_( "Apex" ), QVariant( game_apex2 ) );
    ui->cmbGame->addItem( xorstr_( "Fortnite" ), QVariant( game_fortnite ) );
    ui->cmbGame->addItem( xorstr_( "PUBG Lite" ), QVariant( game_pubglite ) );
    //ui->cmbGame->addItem( xorstr_( "fifa" ), QVariant( game_fifa ) );☺
    ui->cmbGame->addItem( xorstr_( "Overwatch" ), QVariant( game_overwatch ) );
    ui->cmbGame->addItem( xorstr_( "Tarkov" ), QVariant( game_tarkov ) );
    ui->cmbGame->addItem( xorstr_( "Arma" ), QVariant( game_arma ) );
    ui->cmbGame->addItem( xorstr_( "GTA5" ), QVariant( game_gta5 ) );
    ui->cmbGame->addItem( xorstr_( "HW Spoof (EAC)" ), QVariant( hardware_spoof ) );
    ui->cmbGame->addItem( xorstr_( "HW Spoof (BE)" ), QVariant( hardware_spoof2 ) );
    ui->cmbGame->addItem( xorstr_( "Test" ), QVariant( game_test ) );

    ui->cmbCleaner->addItem( xorstr_( "Normally" ), QVariant( cleaner_disabled ) );
    ui->cmbCleaner->addItem( xorstr_( "Regular Clean" ), QVariant( cleaner_regular ) );
    ui->cmbCleaner->addItem( xorstr_( "Max Clean" ), QVariant( cleaner_max ) );


	// for epic only
	// ui->cmbGame->setCurrentIndex( 5 );

	// for orbital only
	// ui->cmbGame->setCurrentIndex( 2 );

	ui->cmbLanguage->addItem( xorstr_( "English" ) );
	ui->cmbLanguage->addItem( xorstr_( "Chinese" ) );
	ui->cmbLanguage->addItem( xorstr_( "Spanish" ) );
	ui->cmbLanguage->addItem( xorstr_( "German" ) );
	ui->cmbLanguage->addItem( xorstr_( "Polish" ) );
	ui->cmbLanguage->addItem( xorstr_( "Croatian" ) );

	ui->cmbLanguage->setItemIcon( 0, QIcon( ":/images/flag_us.png" ) );
	ui->cmbLanguage->setItemIcon( 1, QIcon( ":/images/flag_cn.png" ) );
	ui->cmbLanguage->setItemIcon( 2, QIcon( ":/images/flag_es.png" ) );
	ui->cmbLanguage->setItemIcon( 3, QIcon( ":/images/flag_de.png" ) );
	ui->cmbLanguage->setItemIcon( 4, QIcon( ":/images/flag_pl.png" ) );
	ui->cmbLanguage->setItemIcon( 5, QIcon( ":/images/flag_hr.png" ) );

    //QPalette palette = ui->lblDetectedStatus->palette( );
    //palette.setColor( ui->lblDetectedStatus->foregroundRole( ), Qt::darkMagenta );
    //ui->lblDetectedStatus->setPalette( palette );

    //palette = ui->lblStatusValue->palette( );
    //palette.setColor( ui->lblStatusValue->foregroundRole( ), Qt::darkMagenta );
    //ui->lblStatusValue->setPalette( palette );

	translations.add_language( xorstr_( "us" ), xorstr_( ":/translations/translation_us.txt" ) );
	translations.add_language( xorstr_( "cn" ), xorstr_( ":/translations/translation_cn.txt" ) );
	translations.add_language( xorstr_( "es" ), xorstr_( ":/translations/translation_es.txt" ) );
	translations.add_language( xorstr_( "de" ), xorstr_( ":/translations/translation_de.txt" ) );
	translations.add_language( xorstr_( "pl" ), xorstr_( ":/translations/translation_pl.txt" ) );
	translations.add_language( xorstr_( "hr" ), xorstr_( ":/translations/translation_hr.txt" ) );

	update_status_label( xorstr_( "LABEL_READY" ) );

	// set default translation

	//#ifdef LOADER_CHINA_BUILD
	//	ui->cmbLanguage->setCurrentIndex( 1 );
	//	on_cmbGame_currentIndexChanged( 0 );
   //
    //	ui->lblSite->setText( "<a href=\"https://discord.gg/\">discord.gg/</a>" );
	//	ui->lblSite->setTextFormat( Qt::RichText );
	//	ui->lblSite->setTextInteractionFlags( Qt::TextBrowserInteraction );
	//	ui->lblSite->setOpenExternalLinks( true );
	//#else
        ui->lblSite->setText( "" ); // blank

		ui->lblSite->setTextFormat( Qt::RichText );
		ui->lblSite->setTextInteractionFlags( Qt::TextBrowserInteraction );
		ui->lblSite->setOpenExternalLinks( true );

		//ui->lblSite->hide( );
		//ui->lblHelp->hide( );


	// for error messages

	language_definition* lang = selected_language( ui->cmbLanguage->currentText( ) );

	// check administrative privilege

	if ( !is_elevated( ) )
	{
		QMessageBox::critical( this, xorstr_( "Error" ), lang->get( xorstr_( "MESSAGE_ADMINISTRATOR" ) ) );
		ExitProcess( 0 );
	}

	// check blacklisted applications


	if ( is_blacklisted_app_running( ) || is_blacklisted_window_running( ) || is_fiddler_running( ) )
	{
	    QMessageBox::critical( this, xorstr_( "Error" ), lang->get( xorstr_( "MESSAGE_BLACKLISTED_TOOL" ) ) );
	    m_block_auth = true;
	    ExitProcess( 0 );
	}

	// check if the game is running

	if ( is_game_running( ) )
	{
		QMessageBox::critical( this, xorstr_( "Error" ), lang->get( xorstr_( "MESSAGE_GAME_RUNNING" ) ) );
        ExitProcess( 0 );
	}

	// ignore self signed certificate on the server

    //QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration( );
    //sslConf.setPeerVerifyMode( QSslSocket::VerifyNone );
    //sslConf.setProtocol( QSsl::SslV2 );

    //QSslConfiguration::setDefaultConfiguration( sslConf );

	// check windows version

	if ( get_windows_build( ) < 1803 )
	{
	    QString message = lang->get( xorstr_( "MESSAGE_REQUIRED_WINDOWS_VERSION" ) ) + xorstr_( ": Windows 10 1803-1909" );
	    QMessageBox::critical( this, xorstr_( "Warning" ), message );
        //ExitProcess( 0 );
	}

	// show custom server message

    //if ( !show_server_message( ) )
    //{
        ui->txtServerMessage->setPlainText( lang->get( xorstr_( "MESSAGE_SERVER" ) ) );
    //}

	load_previous_key( );

    int fast_boot_setting = query_fastboot( );

    if ( fast_boot_setting != 0 )
    {
        //QString message = "Fast boot setting:" + QString::number( fast_boot_setting );
        QString message = xorstr_( "Your machine has fast boot turned on." );
        QMessageBox::critical( this, xorstr_( "Ban Risk" ), message );
    }


	VMProtectEnd( );
}

MainWindow::~MainWindow( )
{
    //qDebug( ) << "window destructor";
    curl_global_cleanup( );
	delete ui;
}

void MainWindow::change_label_color( QLabel* label, QColor color )
{
	QPalette palette = label->palette( );
	palette.setColor( label->foregroundRole( ), color );
	label->setPalette( palette );
}

void MainWindow::update_status_label( QString label_definition, unsigned int code )
{
	language_definition* lang = selected_language( ui->cmbLanguage->currentText( ) );

	if ( !lang )
	  return;

	// qDebug( ) << "label:" << label_definition;

	// previous definition & code in case a translation is requested

	this->m_status_definition = label_definition;
	this->m_status_code = code;

	if ( label_definition == xorstr_( "LABEL_ERROR_CODE" ) )
	{
		QString message = lang->get( xorstr_( "LABEL_ERROR_CODE" ) ) + " " + QString::number( code );
		ui->lblStatusValue->setText( message );
	} else {
		ui->lblStatusValue->setText( lang->get( label_definition ) );
	}
}

language_definition* MainWindow::selected_language( const QString& language )
{
	language_definition* lang = nullptr;

	if ( language == xorstr_( "English" ) )
		lang = translations.get_language( xorstr_( "us" ) );
	else if ( language == xorstr_( "Chinese" ) )
		lang = translations.get_language( xorstr_( "cn" ) );
	else if ( language == xorstr_( "German" ) )
		lang = translations.get_language( xorstr_( "de" ) );
	else if ( language == xorstr_( "Spanish" ) )
		lang = translations.get_language( xorstr_( "es" ) );
	else if ( language == xorstr_( "Polish" ) )
		lang = translations.get_language( xorstr_( "pl" ) );
	else if ( language == xorstr_( "Croatian" ) )
		lang = translations.get_language( xorstr_( "hr" ) );

	return lang;
}

void MainWindow::apply_translation( const QString& language )
{
	language_definition* lang = selected_language( language );

	if ( !lang )
	{
		// QMessageBox::critical( this, "Error", "Language not found" );
		return;
	}

	ui->grpAuthentication->setTitle( lang->get( xorstr_( "LABEL_AUTHENTICATION" ) ) );
	ui->grpInformation->setTitle( lang->get( xorstr_( "LABEL_INFORMATION" ) ) );
	ui->grpServerMessage->setTitle( lang->get( xorstr_( "LABEL_SERVER_MESSAGE" ) ) );

	ui->lblSerialKey->setText( lang->get( xorstr_( "LABEL_SERIAL_KEY" ) ) + ":" );
	ui->lblGameType->setText( lang->get( xorstr_( "LABEL_GAME_TYPE" ) ) + ":" );

	//ui->chkHardwareSpoof->setText( lang->get( xorstr_( "LABEL_HARDWARE_SPOOF" ) ) );

	ui->cmdAuthenticate->setText( lang->get( xorstr_( "LABEL_AUTHENTICATE" ) ) );
	ui->cmdQuit->setText( lang->get( xorstr_( "LABEL_QUIT" ) ) );

	ui->lblLanguage->setText( lang->get( xorstr_( "LABEL_LANGUAGE" ) ) + ":" );
	ui->lblLoaderVersion->setText( lang->get( xorstr_( "LABEL_LOADER_VERSION" ) ) + ":" );
	ui->lblLatestVersion->setText( lang->get( xorstr_( "LABEL_LATEST_VERSION" ) ) + ":" );
    //ui->lblCheatStatus->setText( lang->get( xorstr_( "LABEL_CHEAT_STATUS" ) ) + ":" );
    //ui->lblDetectedStatus->setText( lang->get( xorstr_( "LABEL_UNDETECTED" ) ) );
    //ui->lblHoursLeft->setText( lang->get( xorstr_( "LABEL_HOURS_LEFT" ) ) + ":" );
	ui->lblHelp->setText( lang->get( xorstr_( "LABEL_HELP" ) ) + ":" );
	ui->lblStatus->setText( lang->get( xorstr_( "LABEL_STATUS" ) ) + ":" );

	update_status_label( this->m_status_definition, this->m_status_code );
}

__declspec( noinline ) HANDLE MainWindow::launch_dummy( const char* path )
{
	VMProtectBeginMutation( "MainWindow::launch_dummy" );
	////VIRTUALIZER_MUTATE_ONLY_START

    /*
	STARTUPINFOA startup_info = { 0 };
    //startup_info.Verb = "runas";
    //startup_info.ShellExecute=true
	startup_info.cb = sizeof( STARTUPINFO );

	PROCESS_INFORMATION process_info = { 0 };

    // C:\\Windows\\System32\\notepad.exe
    // C:\\Windows\\regedit.exe

    //qDebug( ) << "[-] launching dummy:" << path;

    if ( !CreateProcessA( nullptr, path, nullptr, nullptr, FALSE,

                          CREATE_NEW_PROCESS_GROUP |
                          CREATE_BREAKAWAY_FROM_JOB,
                          nullptr, nullptr, &startup_info, &process_info ) )
	{
        qDebug( ) << "[-] create process failed";
		return 0;
	}

    DWORD dwPid = process_info.dwProcessId;

    //qDebug( ) << "[-] launched" << dwPid;

    //CloseHandle( process_info.hProcess );
    //CloseHandle( process_info.hThread );
    */


    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_DEFAULT;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = L"runas";
    ShExecInfo.lpFile = L"/user:administrator C:\\Windows\\explorer.exe /separate, \"C:\\windows\\system32\\notepad.exe\"";
    ShExecInfo.lpParameters = NULL;
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess,INFINITE);

    /*
    SHELLEXECUTEINFOA sei = { };
    sei.lpVerb  = "runas";
    sei.lpParameters = path;
    sei.hwnd    = NULL;
    sei.lpFile  = NULL;
    sei.nShow   = SW_NORMAL;

    if ( !ShellExecuteExA( &sei ) )
    {
        DWORD dwError = GetLastError( );
        qDebug( ) << "[-] error:" << dwError;
    }
    */



	////VIRTUALIZER_MUTATE_ONLY_END

	VMProtectEnd( );

    return 0;
}

void MainWindow::clean_traces( )
{
	wchar_t temp_path_buffer[MAX_PATH] = { 0 };
    SHGetSpecialFolderPath( nullptr, temp_path_buffer, CSIDL_PROGRAM_FILES, FALSE );


    QString folder1 = QCoreApplication::applicationDirPath( ) + xorstr_( "/local/" );

    //QString folder1 = QString::fromWCharArray( temp_path_buffer ) + xorstr_( "\\Microsoft SQL Server\\120\\Shared\\" );
    //QString folder2 = QString::fromWCharArray( temp_path_buffer ) + xorstr_( "\\MSI\\" );

	// clean new

	QDirIterator it( folder1, QStringList( ) << xorstr_( "*.exe" ) << xorstr_( "*.dll" ), QDir::Files );

	while ( it.hasNext( ) )
	{
		QFileInfo file_info( it.next( ) );

		if ( file_info.fileName( ).length( ) == 26 )
		{
			QFile file( file_info.filePath( ) );
			file.remove( );
		}
	}

}

#include <fstream>

#include "util/befilter_module.h"

bool MainWindow::launch_filter( )
{
    // kill any hung beservice

    unsigned long be_service = get_process_id( xorstr_( L"BEService.exe" ) );

    if ( be_service )
    {
        TerminateProcess( OpenProcess( PROCESS_TERMINATE, false, be_service ), 0 );
        be_service = 0;
    }

    // wait for new service

    // printf( "[-] waiting..\n" );

    while ( !be_service )
    {
        QThread::msleep( 20 );
        QCoreApplication::processEvents( );
        be_service = get_process_id( xorstr_( L"BEService.exe" ) );
    }

    HANDLE process = OpenProcess( PROCESS_ALL_ACCESS, FALSE, be_service );

    if ( !process )
    {
        //printf( "[!] failed to open process\n" );
        return false;
    }

    if ( manual_map( process, befilter_module ) )
        return true;
    else
        return false;
}

bool MainWindow::launch_cheat( authenticator* auth, QString cheat_file, unsigned int game_id )
{
	VMProtectBeginMutation( "MainWindow::launch_cheat" );
	////VIRTUALIZER_MUTATE_ONLY_START

    /*
    qDebug( ) << "[+] launch";

    if ( game_id == game_siege || game_id == game_siege2 )
    {
        if ( load_emulator( ) )
        {
            QMessageBox::information( this, "", "be module loaded" );
        } else {
            QMessageBox::information( this, "", "be module failed to load" );
        }
    }
    */

    if ( game_id == game_overwatch )
	{
		// qDebug( ) << "launching overwatch";

		language_definition* lang = selected_language( ui->cmbLanguage->currentText( ) );

		if ( lang )
			ui->lblStatusValue->setText( lang->get( xorstr_( "LABEL_LAUNCH_GAME" ) ) );
		else
			qDebug( ) << "[!] no lang";

		DWORD pid = get_process_id( xorstr_( L"Overwatch.exe" ) );

		while( !pid )
		{
			QThread::msleep( 20 );
			QCoreApplication::processEvents( );
			pid = get_process_id( xorstr_( L"Overwatch.exe" ) );
		}

        qDebug( ) << "[+] waiting";

        QThread::msleep( 10000 );

		create_loader_challenge( game_id );


        qDebug( ) << "[+] i" << cheat_file;

		if ( !inject_dll( pid, cheat_file.toStdString( ).c_str( ) ) )
		{
			QMessageBox::critical( this, xorstr_( "Error" ), xorstr_( "Product failed to load, please send developers this error code: 0x98" ) );
		}


		//delete_usn_journal( xorstr_( "\\\\.\\C:" ) );
		clean_other_caches( );
        return true;

    } else if ( game_id == game_gta5 ) {

        // prompt to launch game

        language_definition* lang = selected_language( ui->cmbLanguage->currentText( ) );

        if ( lang )
            ui->lblStatusValue->setText( lang->get( xorstr_( "LABEL_LAUNCH_GAME" ) ) );
        else
            qDebug( ) << "[!] no lang";

        DWORD pid = get_process_id( xorstr_( L"GTA5.exe" ) );

        while( !pid )
        {
            QThread::msleep( 20 );
            QCoreApplication::processEvents( );
            pid = get_process_id( xorstr_( L"GTA5.exe" ) );
        }

        HANDLE handle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );

        if ( !handle )
        {
            QMessageBox::critical( this, xorstr_( "Send this error to developers" ), xorstr_( "process handle invalid: " ) + QString::number( pid ) );
            return false;
        }

        // gta5 doesn't support manual mapping


        QFile file( cheat_file );
        file.open( QIODevice::WriteOnly );
        qint64 bytes_written = file.write( auth->m_cheat_byte_array.data( ), static_cast<int64_t>( auth->m_cheat_byte_array.size( ) ) );
        file.close( );

        ui->lblStatusValue->setText( lang->get( xorstr_( "LABEL_LAUNCH_GAME" ) ) );
        //QThread::msleep( 6000 );

        create_loader_challenge( game_id );
        inject_dll( pid, cheat_file.toStdString( ).c_str( ) );

    } else if ( game_id == game_rust2 ) {

        /*
        DWORD pid = launch_dummy( xorstr_( "C:\\Windows\\System32\\notepad.exe" ) );

        if ( !pid )
        {
            QMessageBox::critical( this, xorstr_( "Error" ), xorstr_( "Product failed to load process failed to start error code: 0x96" ) );
        }
        */


        HANDLE handle = launch_dummy( xorstr_( "C:\\Windows\\System32\\notepad.exe" ) );

        if ( !handle )
        {
            QMessageBox::critical( this, xorstr_( "Send this error to developers" ), xorstr_( "process handle invalid: " ) + QString::number( 0 ) );
            return false;
        }

        create_loader_challenge( game_id );
        manual_map( handle, reinterpret_cast<BYTE*>( auth->m_cheat_byte_array.data( ) ) );

        /*
    } else if ( game_id == game_rust2 ) {

        qDebug( ) << "[-] mapping";

        std::vector<uint8_t> file_data;
        std::ifstream test_file( cheat_file.toStdString( ).c_str( ), std::ios_base::in | std::ios_base::binary );

        unsigned char ch = test_file.get( );

        while ( test_file.good( ) )
        {
            file_data.push_back( ch );
            ch = test_file.get( );
        }

        // C:\Windows\regedit.exe

        //DWORD search_indexer = get_process_id( xorstr_( L"SearchIndexer.exe" ) );

        //if ( !search_indexer )
        //{
        //    QMessageBox::critical( this, xorstr_( "Send this error to developers" ), xorstr_( "indexer not available" ) );
        //    return false;
        //}

        create_loader_challenge( game_id );
        DWORD pid = launch_dummy( );

        QThread::msleep( 2000 );
        QCoreApplication::processEvents( );

        HANDLE handle = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );

        if ( !handle )
        {
            QMessageBox::critical( this, xorstr_( "Send this error to developers" ), xorstr_( "indexer handle invalid: " ) + QString::number( pid ) );
            return false;
        }

        if ( !manual_map( handle, file_data.data( ) ) )
        {
            QMessageBox::critical( this, xorstr_( "Send this error to developers" ), xorstr_( "failed to map: " ) + QString::number( file_data.size( ) ) );
            return false;
        }

        QMessageBox::information( this, xorstr_( "Loaded" ), xorstr_( "Loaded successfully" ) );

        //

        clean_other_caches( );
        return true;
        */

	} else if ( cheat_file.contains( xorstr_( ".dll" ) ) && game_id != game_overwatch ) {


       // HANDLE hFile = CreateFileA( cheat_file.toStdString( ).c_str( ), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
       // DWORD error = GetLastError( );

      //  if ( hFile == INVALID_HANDLE_VALUE )
       //     qDebug( ) << "[!] failed to open file hfile=" << hFile << "error=" << error;

        //HANDLE handle = launch_dummy( xorstr_( "C:\\Windows\\System32\\notepad.exe" ) );


        STARTUPINFOA startup_info = { 0 };
        //startup_info.Verb = "runas";
        //startup_info.ShellExecute=true
        startup_info.cb = sizeof( STARTUPINFO );

        PROCESS_INFORMATION process_info = { 0 };

        // C:\\Windows\\System32\\notepad.exe
        // C:\\Windows\\regedit.exe

        //qDebug( ) << "[-] launching dummy:" << path;

        if ( !CreateProcessA( nullptr, (LPSTR)"C:\\Windows\\System32\\cmd.exe", nullptr, nullptr, FALSE,
                              CREATE_NEW_PROCESS_GROUP |
                              CREATE_BREAKAWAY_FROM_JOB,
                              nullptr, nullptr, &startup_info, &process_info ) )
        {
            QMessageBox::critical( this, xorstr_( "Error" ), xorstr_( "Failed to launch, please send developers this code: 0x5001" ) );
            return 0;
        }

        DWORD pid = process_info.dwProcessId;

        //qDebug( ) << "launched:" << pid;

		//create_challenge( );
		create_loader_challenge( game_id );

        //qDebug( ) << "module:" << cheat_file;

        QFile file( cheat_file );
        file.open( QIODevice::WriteOnly );
        file.write( auth->m_cheat_byte_array.data( ), static_cast<int64_t>( auth->m_cheat_byte_array.size( ) ) );
        file.close( );


		if ( !inject_dll( pid, cheat_file.toStdString( ).c_str( ) ) )
		{
			clean_traces( );
			QMessageBox::critical( this, xorstr_( "Error" ), xorstr_( "Failed to launch, please send developers this code: 0x6001" ) );
			return false;
		} else {
            //clean_traces( );
            //delete_usn_journal( xorstr_( "\\\\.\\C:" ) );
            //clean_other_caches( );
		}




	} else if ( cheat_file.contains( xorstr_( ".exe" ) ) ) {

		STARTUPINFOA startup_info = { 0 };
		startup_info.cb = sizeof( STARTUPINFO );

		PROCESS_INFORMATION process_info = { 0 };

		create_loader_challenge( game_id );

		QString parameter;

		BOOL created = CreateProcessA( cheat_file.toStdString( ).c_str( ),
			const_cast<LPSTR>( parameter.toStdString( ).c_str( ) ),
			nullptr,
			nullptr,
			TRUE,
			NULL,
			nullptr,
			nullptr,
			&startup_info,
			&process_info );

        //DWORD last_error = 0;
        //QMessageBox::information( nullptr, "Screenshot message", "created, gle:" + QString::number( last_error ) );

		if ( !created )
		{
			return false;
		}

		CloseHandle( process_info.hProcess );
		CloseHandle( process_info.hThread );

        //clean_traces( );
        //delete_usn_journal( xorstr_( "\\\\.\\C:" ) );
        //clean_other_caches( );

	}

	////VIRTUALIZER_MUTATE_ONLY_END


	//HANDLE process_handle = OpenProcess( PROCESS_ALL_ACCESS, false, process_info.dwProcessId );
	//DllMapper* mapper = new DllMapper( Process( process_handle ) );
	//mapper->map_module( cheat_file.toStdWString( ).c_str( ) );
	//delete mapper;

	VMProtectEnd( );
	//

    return true;
}

__declspec( noinline ) bool MainWindow::launch_exe_blocking( const QString& path, bool block )
{

    STARTUPINFOA startup_info;
    RtlSecureZeroMemory( &startup_info, sizeof( startup_info ) );
    startup_info.cb = sizeof( STARTUPINFO );

    PROCESS_INFORMATION process_info;
    RtlSecureZeroMemory( &process_info, sizeof( process_info ) );

    if ( CreateProcessA( path.toStdString( ).c_str( ), nullptr, nullptr, nullptr, TRUE, NULL, nullptr, nullptr, &startup_info, &process_info ) )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 1000 );

        this->activateWindow( );
        this->raise( );

        if ( block )
            WaitForSingleObject( process_info.hProcess, INFINITE );

        CloseHandle( process_info.hProcess );
        CloseHandle( process_info.hThread );
        return true;
    }

    return false;
}

__declspec( noinline ) bool MainWindow::launch_cleaner( )
{
    VMProtectBeginMutation( "MainWindow::launch_cleaner" );

    unsigned int cleaner_level = ui->cmbCleaner->currentIndex( );

    if ( cleaner_level == cleaner_disabled )
        return true;

    //qDebug( ) << "[-] launching cleaner level:" << ui->cmbCleaner->currentIndex( );

    QFileInfo info( QCoreApplication::applicationFilePath( ) );

    if ( cleaner_level == cleaner_regular )
    {
        //qDebug( ) << "[-] launching" + info.dir( ).path( ) + "/clean/hw1.exe";

        ui->lblStatusValue->setText( xorstr_( "Starting cleaner [0/1]" ) );
        change_label_color( ui->lblStatusValue, Qt::darkMagenta );

        QString app1 = info.dir( ).path( ) + xorstr_( "/clean/hw1.exe" );
        launch_exe_blocking( app1, true );
    }

    if ( cleaner_level == cleaner_max )
    {
        //QMessageBox::information( this, "Cleaners", "Click " );

        clean_prefetch( );
        delete_usn_journal( xorstr_( "\\\\.\\C:" ) );

        ui->lblStatusValue->setText( xorstr_( "Waiting for cleaners to run [1/3]" ) );
        change_label_color( ui->lblStatusValue, Qt::darkMagenta );

        QString app1 = info.dir( ).path( ) + xorstr_( "/clean/hw1.exe" );
        launch_exe_blocking( app1, true );

        ui->lblStatusValue->setText( xorstr_( "Waiting for cleaners to run [2/3]" ) );
        change_label_color( ui->lblStatusValue, Qt::darkMagenta );

        QString app2 = info.dir( ).path( ) + xorstr_( "/clean/gs1.exe" );
        launch_exe_blocking( app2, false );

        ui->lblStatusValue->setText( xorstr_( "Waiting for cleaners to run [3/3]" ) );
        change_label_color( ui->lblStatusValue, Qt::darkMagenta );

        QString app3 = info.dir( ).path( ) + xorstr_( "/clean/gs2.exe" );
        launch_exe_blocking( app3, false );

    }

    VMProtectEnd( );

    return true;
}

__declspec( noinline ) bool MainWindow::load_emulator( )
{
    VMProtectBeginMutation( "MainWindow::load_emulator" );

    util::memory_struct emulator_driver;
    util::curl_http_get_binary( xorstr_( "http://127.0.0.1/access/driver200.bin" ), emulator_driver );

    //qDebug( ) << "[+] downloaded";

    QByteArray decrypted_data = decrypt( QByteArray::fromRawData( emulator_driver.memory, ( int )emulator_driver.size ) );

    if( !load_driver( decrypted_data ) )
    {
        return false;
    }

    VMProtectEnd( );
    return true;
}

__declspec( noinline ) bool MainWindow::show_server_message( )
{
	VMProtectBeginMutation( "MainWindow::show_server_message" );
	////VIRTUALIZER_MUTATE_ONLY_START

    return false;

	//QNetworkAccessManager manager;

    QString url;
    QString data;

	//#ifdef LOADER_CHINA_BUILD
	//	url = xorstr_( "https://glot.io/snippets/ff3chup969/raw/message.value" );
	//#else
		url = xorstr_( "https://glot.io/snippets/ff2avrzmzf/raw/message.value" ); // clubhouse
		//url = "https://glot.io/snippets/fftw4kyzig/raw/main.txt";

	//#endif

    data = util::curl_http_get( url );

    ui->txtServerMessage->setPlainText( data );

	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END

	return true;
}

bool MainWindow::check_secure_boot( )
{
	//NtQuerySystemInformation( SystemBootEnvironmentInformation, &sbei, sizeof(sbei), &returnLength );
	//if (sbei.FirmwareType == FirmwareTypeUefi) {
	//	bSecureBoot = FALSE;
	//	if (QuerySecureBootState(&bSecureBoot)) {

	return true;
}

void MainWindow::save_previous_key( )
{
	QFile file( QCoreApplication::applicationDirPath( ) + xorstr_( "/config/lastkey.txt" ) );

	if ( file.open( QIODevice::ReadWrite ) )
	{
		QTextStream stream( &file );
		stream << ui->txtSerial->text( ) << ":" << ui->cmbGame->currentData( Qt::UserRole ).toUInt( );
	}
}

void MainWindow::load_previous_key( )
{
	QFile file( QCoreApplication::applicationDirPath( ) + xorstr_( "/config/lastkey.txt" ) );

	if ( !file.open( QIODevice::ReadOnly ) )
		return;

	QTextStream in( &file );
	QStringList pieces = in.readLine( ).split( ":" );

	if ( pieces.count( ) != 2 )
	{
		// qDebug( ) << "[+] count: " << pieces.count( );
		return;
	}

	ui->txtSerial->setText( pieces.at( 0 ) );
	//ui->cmbGame->setCurrentIndex( pieces.at( 1 ).toInt( ) );
}

void MainWindow::clean_prefetch( )
{
	VMProtectBeginMutation( "clean_prefetch" );

	QString windows_directory = QString::fromUtf8( qgetenv( "windir" ) );

	QDirIterator it( windows_directory + "\\prefetch", QStringList( ) << xorstr_( "*.pf" ), QDir::Files );

	while ( it.hasNext( ) )
	{
		QFileInfo file_info( it.next( ) );
		QFile file( file_info.filePath( ) );
		file.remove( );
	}

	VMProtectEnd( );
}

void MainWindow::on_cmbLanguage_currentTextChanged( const QString& arg1 )
{
	if ( translations.language_count( ) == 0 )
		return;

	apply_translation( arg1 );
}

void MainWindow::on_cmdAuthenticate_clicked( )
{
	VMProtectBeginMutation( "MainWindow::cmdAuthenticate" );
	////VIRTUALIZER_MUTATE_ONLY_START

	language_definition* lang = selected_language( ui->cmbLanguage->currentText( ) );

	if ( !lang )
	  return;

	if ( is_blacklisted_app_running( ) || is_blacklisted_window_running( ) || is_fiddler_running( ) )
	{
	    QMessageBox::critical( this, xorstr_( "Error" ), lang->get( xorstr_( "MESSAGE_BLACKLISTED_TOOL" ) ) );
	    ExitProcess( 0 );
	}

	update_status_label( xorstr_( "LABEL_CONNECTING" ) );

	QString serial_key = ui->txtSerial->text( );
	unsigned int game_id = ui->cmbGame->currentData( Qt::UserRole ).toUInt( );
	authenticator auth( serial_key, game_id );

	// auth.get_monitor_serial( );

	ui->progress->setValue( 1 );
	//qDebug( ) << "progress:" << ui->progress->value( );

	if ( !auth.validate_key_format( ) )
	{
		update_status_label( xorstr_( "LABEL_SERIAL_INVALID" ) );
		change_label_color( ui->lblStatusValue, Qt::red );
		return;
	}

    ui->lblStatusValue->setText( xorstr_( "Downloading please wait.." ) );
    change_label_color( ui->lblStatusValue, Qt::darkMagenta );

    //if ( !auth.validate_key_against_cache( ) )
    //{
    //	update_status_label( xorstr_( "LABEL_ERROR_CODE" ), ui_failure_cache_validation );
    //	change_label_color( ui->lblStatusValue, Qt::red );
    //	return;
    //}

    //ui->lblStatusValue->setText( xorstr_( "Downloading metadata (2/2)" ) );
    //change_label_color( ui->lblStatusValue, Qt::darkMagenta );

	ui->progress->setValue( 2 );
	//qDebug( ) << "progress:" << ui->progress->value( );

	unsigned int code = auth.validate_key( );

	ui->progress->setValue( 3 );
	//qDebug( ) << "progress:" << ui->progress->value( );

	// ** other codes **

	switch( code )
	{
	case ui_failure_cache_validation:
	case ui_failure_invalid_serial:
		ui->progress->setValue( 0 );
		update_status_label( xorstr_( "LABEL_SERIAL_INVALID" ) );
		change_label_color( ui->lblStatusValue, Qt::red );
		return;
	case ui_failure_invalid_loader_version:
		ui->progress->setValue( 0 );
		QMessageBox::critical( nullptr, xorstr_( "Error" ), lang->get( xorstr_( "MESSAGE_LOADER_OUT_OF_DATE" ) ) );
		update_status_label( xorstr_( "LABEL_ERROR_CODE" ), code );
		change_label_color( ui->lblStatusValue, Qt::red );
		return;
	case ui_failure_invalid_computerid:
		ui->progress->setValue( 0 );
		update_status_label( xorstr_( "LABEL_SERIAL_REGISTERED_TO_ANOTHER_USER" ) );
		change_label_color( ui->lblStatusValue, Qt::red );
		return;
	case ui_failure_expired_serial:
		ui->progress->setValue( 0 );
		update_status_label( xorstr_( "LABEL_SERIAL_EXPIRED" ) );
		change_label_color( ui->lblStatusValue, Qt::red );
		return;
	case ui_failure_maintenance:
		ui->progress->setValue( 0 );
		update_status_label( xorstr_( "LABEL_CHEAT_IS_UNDER_MAINTENANCE" ) );
		change_label_color( ui->lblStatusValue, Qt::darkBlue );
		return;
	}

	// ** auth was not successful, hide some return code text, show integer value only **

	if ( code != ui_success )
	{
		ui->progress->setValue( 0 );
		update_status_label( xorstr_( "LABEL_ERROR_CODE" ), code );
		change_label_color( ui->lblStatusValue, Qt::red );
		return;
	}

	// ** download files **


	unsigned download_result_code = auth.download_files( );

    ui->progress->setValue( 4 );


	if ( download_result_code != ui_success )
	{  
		ui->progress->setValue( 0 );
		update_status_label( xorstr_( "LABEL_ERROR_CODE" ), download_result_code );
		change_label_color( ui->lblStatusValue, Qt::red );
	}

    ui->lblStatusValue->setText( xorstr_( "Launching.." ) );
    change_label_color( ui->lblStatusValue, Qt::darkMagenta );
	ui->progress->setValue( 5 );
    QCoreApplication::processEvents( QEventLoop::AllEvents, 1000 );

	// ** map driver **

    //qDebug( ) << "[-] mapping";

    //QFile file( "C:\\temp\\loader_output.sys" );
    //file.open( QIODevice::WriteOnly );
    //qint64 bytes_written = file.write( auth.driver_byte_array( ) );
    //file.close( );


    //HANDLE intel_driver = intel_driver::Load( );
    //qDebug( ) << "kdmapper map:" << kdmapper::MapDriver( intel_driver, auth.driver_byte_array( ).data( ) );

 //   if ( game_id == game_rust || game_id == game_rust2 || game_id == game_siege || game_id == game_siege2 || game_id == hardware_spoof )
//    {
 //       qDebug( ) << "[-] using kdmapper";
//
  //      HANDLE intel_driver = intel_driver::Load( );
 //       kdmapper::MapDriver( intel_driver, auth.driver_byte_array( ).data( ) );
//
  //  } else {

        if( !load_driver( auth.driver_byte_array( ) ) )
        {
            ui->progress->setValue( 0 );
            update_status_label( xorstr_( "LABEL_ERROR_CODE" ), ui_failure_mapper_exception );
            change_label_color( ui->lblStatusValue, Qt::red );
        }
   // }


    //qDebug( ) << "[-] mapped";

	ui->progress->setValue( 6 );


	// ** spoof if requested **

	/*
	if ( ui->chkHardwareSpoof->isChecked( ) )
	{
		if ( verify_hook( ) )
		{
			uint64_t code = spoof_drives( );

			if ( code != 0 )
				QMessageBox::information( this, "Warning", "HW serials have not been changed. Please send this code to the developer: " + QString::number( code, 16 ) );
		}
	}*/

	// ** run cheat **

    //update_status_label( xorstr_( "LABEL_SERIAL_VALID" ) );
    //change_label_color( ui->lblStatusValue, Qt::darkMagenta );

    save_previous_key( );

    qDebug( ) << "[-] launching";

    if ( game_id != hardware_spoof )
    {
        // 1. launch cheat

        if ( !launch_cheat( &auth, auth.cheat_file( ), game_id ) )
        {
            ui->lblStatusValue->setText( xorstr_( "Cheat failed to load. " ) );
            change_label_color( ui->lblStatusValue, Qt::red );
            return;
        }

        ui->progress->setValue( 7 );

        // 2. launch cleaner

        launch_cleaner( );

        // 3. launch wait

        this->activateWindow( );
        this->raise( );

        // 4. if BE game launch befilter

       // if ( game_id == game_siege2 || game_id == game_dayz || game_id == game_tarkov )
        //{

            //qDebug( ) << "[-] launch befilter";

           // ui->lblStatusValue->setText( "Do not close loader, launch game" );
           // change_label_color( ui->lblStatusValue, Qt::darkMagenta );

           // launch_filter( );

       // } else {

        // Else wait and close

            QString message = lang->get( xorstr_( "LABEL_WAIT" ) );
            QTime wait_time = QTime::currentTime( ).addSecs( 9 );

            while ( QTime::currentTime( ) < wait_time )
            {
              qint64 milliseconds = QTime::currentTime( ).msecsTo( wait_time );
              qint64 count_down = milliseconds / 10;
              float formatted_count_down = count_down / 100.f;


              ui->lblStatusValue->setText( message + " " + QString::number( formatted_count_down ) + "s" );
              change_label_color( ui->lblStatusValue, Qt::darkMagenta );
              QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
            }

       // }
    }

     ui->progress->setValue( 8 );
     ui->lblStatusValue->setText( xorstr_( "Loaded successfully." ) );
     change_label_color( ui->lblStatusValue, Qt::darkMagenta );


	if ( game_id < 98 )
    {
        ui->lblStatusValue->setText( xorstr_( "Loaded successfully. Closing." ) );
        change_label_color( ui->lblStatusValue, Qt::darkMagenta );
        TerminateProcess( GetCurrentProcess( ), 0 );
        //__fastfail( 0 );
        //QTimer::singleShot( 2000, this, SLOT( on_close_requested( ) ) );
    }


	VMProtectEnd( );
	////VIRTUALIZER_MUTATE_ONLY_END
}

void MainWindow::on_cmdQuit_clicked( )
{
	QApplication::quit( );
}

void MainWindow::on_cmbGame_currentIndexChanged( int index )
{
	Q_UNUSED( index );


    if ( ui->cmbGame->currentData( Qt::UserRole ).toUInt( ) == hardware_spoof )
	{
        ui->cmbCleaner->setEnabled( false );
        ui->cmbCleaner->setCurrentIndex( cleaner_disabled );
	} else {
        ui->cmbCleaner->setEnabled( true );
	}


}

void MainWindow::on_close_requested( )
{
	ExitProcess( 0 );
}

void MainWindow::on_cmdOptions_clicked()
{
	dlg_config dialog;
	dialog.setModal( true );

	if ( dialog.exec( ) != QDialog::Accepted )
		return;
}

void MainWindow::on_cmdHardware_clicked()
{
	dlg_hardware dialog;
	dialog.setModal( true );

	if ( dialog.exec( ) != QDialog::Accepted )
		return;
}
