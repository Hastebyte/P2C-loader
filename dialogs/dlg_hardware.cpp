#include "dlg_hardware.h"
#include "ui_dlg_hardware.h"

QString dlg_hardware::get_smartctl( unsigned int drive_number )
{
    QString serial_number = xorstr_( "Failed to query" );

    QString device_name = xorstr_( "\\\\.\\PhysicalDrive" ) + QString::number( drive_number );

    HANDLE hDrive = CreateFileA( device_name.toStdString( ).c_str( ), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if ( hDrive == INVALID_HANDLE_VALUE )
    {
        //QMessageBox::information( this, xorstr_( "Error" ), xorstr_( "Failed to open PhysicalDrive0" ) );
        return serial_number;
    }

    SENDCMDINPARAMS command;
    RtlSecureZeroMemory( &command, sizeof( command ) );

    command.irDriveRegs.bCommandReg	= ID_CMD;
    command.irDriveRegs.bCylLowReg	= SMART_CYL_LOW;
    command.irDriveRegs.bCylHighReg	= SMART_CYL_HI;
    command.cBufferSize				= IDENTIFY_BUFFER_SIZE;
    command.bDriveNumber			= static_cast<UCHAR>( 0 );

    SIZE_T result_size = sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE;

    PSENDCMDOUTPARAMS result
        = reinterpret_cast<PSENDCMDOUTPARAMS>( malloc( result_size ) );

    if ( !result )
    {
        //qDebug( ) << "[!] failed to allocate memory";
        return serial_number;
    }

    DWORD bytes_returned = 0;

    if ( DeviceIoControl( hDrive, SMART_RCV_DRIVE_DATA, &command, sizeof( SENDCMDINPARAMS ), result, static_cast<uint32_t>( result_size ), &bytes_returned, nullptr ) )
    {
        WORD* pIdSector = ( WORD *)( PIDENTIFY_DATA )result->bBuffer ;

        char szSerialNumber[100] = "" ;
        for (int index = 10, position=0; index <= 19; index++)
        {
            szSerialNumber[position] = ( char )( pIdSector[index] / 256 );
            position++;

            szSerialNumber[position] = ( char )( pIdSector[index] % 256 );
            position++;
        }

        QString unformatted = szSerialNumber;
        serial_number = unformatted.trimmed( );
    }

    free( result );
    CloseHandle( hDrive );

    return serial_number;
}

void dlg_hardware::get_mac( )
{
    HANDLE hNIC = CreateFile( L"\\\\?\\GLOBALROOT\\Device\\NDMP0", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if ( hNIC == INVALID_HANDLE_VALUE )
    {
        qDebug( ) << "[-] failed to open NDMP0";
        //QMessageBox::information( this, xorstr_( "Error" ), xorstr_( "Failed to open NDMP0" ) );
        return;
    }

    CloseHandle( hNIC );
}

dlg_hardware::dlg_hardware( QWidget* parent ) : QDialog( parent ), ui( new Ui::dlg_hardware )
{
	ui->setupUi( this );

	QStringList labels;
	labels.push_back( "Device" );
	labels.push_back( "Property" );
	labels.push_back( "Value" );
	labels.push_back( "Action" );

	ui->treeDisk->setColumnCount( labels.count( ) );
	ui->treeDisk->setHeaderLabels( labels );

	ui->treeDisk->setContextMenuPolicy( Qt::CustomContextMenu );
	ui->treeDisk->setSelectionBehavior( QAbstractItemView::SelectRows );
	ui->treeDisk->setSortingEnabled( false );
	ui->treeDisk->setAlternatingRowColors( true );

	ui->treeDisk->header( )->setSectionResizeMode( QHeaderView::ResizeToContents );
	ui->treeDisk->header( )->setStretchLastSection( true );
	ui->treeDisk->header( )->hide( );


    // query hardware

    m_serial_disk0 = xorstr_( "Failed to query" );
    m_serial_nic0  = xorstr_( "Failed to query" );

    //get_smartctl( );
    //get_mac( );



	QStringList headers;
	headers.push_back( "Device Type" );
	headers.push_back( "Serial Number" );

	ui->treeDisk->setColumnCount( headers.count( ) );
	ui->treeDisk->setHeaderLabels( headers );

    for ( int i = 0; i < 6; i++ )
    {
        QString serial = get_smartctl( i );

        if ( serial == "Failed to query" )
            continue;

        QTreeWidgetItem* pDiskNode = new QTreeWidgetItem( ui->treeDisk );
        pDiskNode->setIcon( 0, QIcon( ":/images/drive.png" ) );
        pDiskNode->setData( 0, Qt::UserRole, 0 );
        pDiskNode->setText( 0, "PhysicalDrive" + QString::number( i ) );
        pDiskNode->setIcon( 1, QIcon( ":/images/key-solid.png" ) );
        pDiskNode->setText( 1, serial );
    }





	ui->treeDisk->expandAll( );

    QString runtime_lib_path = QString::fromUtf8( qgetenv( xorstr_( "windir" ) ) );
    runtime_lib_path += xorstr_( "\\system32" );
    runtime_lib_path += xorstr_( "\\msvcp140.dll" );

    //qDebug( ) << runtime_lib_path;

    QFileInfo msvc_file( runtime_lib_path );

    if ( msvc_file.exists( ) && msvc_file.isFile( ) )
    {
        ui->lblRuntimes->setText( xorstr_( "Found" ) );
    } else {
        ui->lblRuntimes->setText( xorstr_( "Not found" ) );
    }

    if ( query_fastboot( ) != 0 )
    {
        ui->lblFastboot->setText( xorstr_( "Enabled (Ban Risk)" ) );
    } else {
        ui->lblFastboot->setText( xorstr_( "Disabled (Good)" ) );
    }
}

dlg_hardware::~dlg_hardware( )
{
	delete ui;
}

bool dlg_hardware::get_system_disk_information( )
{
	// Reference:
	//
	// https://docs.microsoft.com/en-us/windows/desktop/cimwin32prov/win32-diskdrive
	// https://docs.microsoft.com/en-us/windows/desktop/cimwin32prov/win32-logicaldisk

	// MediaType:
	//  Removable media other than floppy (11)
	//  Fixed hard disk media (12)

	IWbemLocator* pLocator = nullptr;
	IWbemServices* pServices = nullptr;
	IEnumWbemClassObject* pEnumerator = nullptr;

	if ( !wmi_connect( pLocator, pServices ) ||
		 !wmi_execute( pServices, L"Select * from Win32_DiskDrive", pEnumerator ) )
	{
		qDebug( ) << "[!] Failed to call wmi_connect( )";
		return false;
	}

	IWbemClassObject* pClassObject = nullptr;

	// flush old list

	m_disk.clear( );

	// populate new

	while ( wmi_get_next_result( pEnumerator, pClassObject ) )
	{
		_variant_t vt;

		disk_info* info		= new disk_info;
		info->index			= wmi_get_result_long( pClassObject, vt, L"index" );
		info->device_id		= QString::fromWCharArray( wmi_get_result_wide_string( pClassObject, vt, L"DeviceID" ) );
		info->model			= QString::fromWCharArray( wmi_get_result_wide_string( pClassObject, vt, L"Model" ) );
		info->serial		= QString::fromWCharArray( wmi_get_result_wide_string( pClassObject, vt, L"SerialNumber" ) );
		info->size			= wmi_get_result_uint64( pClassObject, vt, L"Size" );

		m_disk.push_back( info );

		VariantClear( &vt );
		pClassObject->Release( );
	}

	pEnumerator->Release( );
	pServices->Release( );
	pLocator->Release( );

	/* Partition 0:
		Get-WmiObject -Query "ASSOCIATORS OF {Win32_DiskPartition.DeviceID='Disk #0, Partition #0'} WHERE AssocClass = Win32_LogicalDiskToPartition"
		DeviceID     : E:
		DriveType    : 3
		ProviderName :
		FreeSpace    : 38014685184
		Size         : 120031539200
		VolumeName   : SSD */

	return true;
}



bool dlg_hardware::populate_tree( )
{
	ui->treeDisk->clear( );

	// sort array

	// qDebug( ) <<"sorting..";

	qSort( m_disk.begin( ), m_disk.end( ), ForwardLessThen<disk_info>( ) );

	// show details

	foreach ( disk_info* disk, m_disk )
	{
		QTreeWidgetItem* pDiskNode = new QTreeWidgetItem( ui->treeDisk );
		//QFont font = pDiskNode->font( 0 );
		//font.setBold( true );
		//pDiskNode->setFont( 0, font );
		pDiskNode->setText( 0, disk->device_id );
		pDiskNode->setIcon( 0, QIcon( ":/images/drive.png" ) );
		pDiskNode->setData( 0, Qt::UserRole, reinterpret_cast<uintptr_t>( pDiskNode ) );
		pDiskNode->setText( 1, disk->serial );
		pDiskNode->setCheckState( 0, Qt::Unchecked );
		//ui->treeDisk->setItemWidget( pDiskNode, 3, new QPushButton( "Change" ) );

		// Add children

		QTreeWidgetItem* pItemModel = new QTreeWidgetItem( pDiskNode );
		pItemModel->setText( 0, "Model" );
		pItemModel->setIcon( 0, QIcon( ":/images/bullet_black.png" ) );
		pItemModel->setText( 1, disk->model );
		pItemModel->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
		// pItemModel->setCheckState( 0, Qt::Unchecked );

	}

	ui->treeDisk->expandAll( );

	return true;
}

