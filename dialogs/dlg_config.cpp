#include "dlg_config.h"
#include "ui_dlg_config.h"
#include "dlg_key.h"

dlg_config::dlg_config( QWidget* parent ) : QDialog( parent ), ui( new Ui::dlg_config )
{
	ui->setupUi( this );
	m_settings = nullptr;
	populate_config_list( );
}

dlg_config::~dlg_config( )
{
	if ( m_settings )
		delete m_settings;

	delete ui;
}

void dlg_config::create_header( )
{
	ui->tblOptions->setSelectionBehavior( QAbstractItemView::SelectRows );
	ui->tblOptions->horizontalHeader( )->setDefaultAlignment( Qt::AlignLeft );
	ui->tblOptions->verticalHeader( )->setDefaultSectionSize( ui->tblOptions->verticalHeader( )->fontMetrics( ).height( ) + 2 );
	ui->tblOptions->verticalHeader( )->hide( );
	ui->tblOptions->setSortingEnabled( false );
	ui->tblOptions->setColumnCount( 3 );
	ui->tblOptions->setHorizontalHeaderItem( 0, new QTableWidgetItem( "Name" ) );
	ui->tblOptions->setHorizontalHeaderItem( 1, new QTableWidgetItem( "Value" ) );
	ui->tblOptions->setHorizontalHeaderItem( 2, new QTableWidgetItem( "Change" ) );
}


void dlg_config::populate_config_list( )
{
	// qDebug( ) << "[+] populating list";

	QDir directory( QCoreApplication::applicationDirPath( ) + "/config/" );
	QStringList config_files = directory.entryList( QStringList( ) << "*.ini" << "*.INI", QDir::Files );

	foreach( QString file, config_files )
	{
		ui->cmbConfigFiles->addItem( file );
	}
}

void dlg_config::on_cmbConfigFiles_currentIndexChanged( const QString& text )
{
	parse_config_file( text );
}

void dlg_config::parse_config_file( const QString& file_name )
{
	ui->tblOptions->clear( );
	ui->tblOptions->setRowCount( 0 );
	create_header( );

	QString path = QCoreApplication::applicationDirPath( ) + "/config/" + file_name;

	// qDebug( ) << "[-] opening:" << path;

	if ( m_settings )
		delete m_settings;

	m_settings = new QSettings( path, QSettings::IniFormat );

	// qDebug( ) << "[-] number of keys:" << settings.allKeys( ).length( );

	// add row lambda

	auto add_row = [this]( QString key, unsigned int numerical_value )
	{
		int row = ui->tblOptions->rowCount( );
		ui->tblOptions->insertRow( row );

		QTableWidgetItem* key_item = new QTableWidgetItem( key.trimmed( ) );
		key_item->setData( Qt::UserRole, numerical_value );
		ui->tblOptions->setItem( row, 0, key_item );
		ui->tblOptions->setItem( row, 1, new QTableWidgetItem( get_key_name( numerical_value ) ) );
		//ui->tblOptions->setData( Qt::UserRole, myData);

		QWidget* pWidget = new QWidget( );
		QPushButton* cmdEdit = new QPushButton( );
		cmdEdit->setText( "Edit" );

		QHBoxLayout* pLayout = new QHBoxLayout( pWidget );
		pLayout->addWidget( cmdEdit );
		pLayout->setAlignment( Qt::AlignCenter );
		pLayout->setContentsMargins( 0, 0, 0, 0 );
		pWidget->setLayout( pLayout );

		connect( cmdEdit, SIGNAL( clicked( ) ), this, SLOT( change_clicked( ) ) );
		ui->tblOptions->setCellWidget( row, 2, pWidget );
	};

	// ini file loop

	foreach ( const QString& group, m_settings->childGroups( ) )
	{
		m_settings->beginGroup( group );

		foreach ( const QString& key, m_settings->childKeys( ) )
		{
			QString string_value = m_settings->value( key ).toString( );
			unsigned int numerical_value = string_value.toUInt( Q_NULLPTR, 16 );

			add_row( group + "/" + key, numerical_value );
		}

		m_settings->endGroup( );

	}

	ui->tblOptions->horizontalHeader( )->setSectionResizeMode( QHeaderView::ResizeToContents );
	ui->tblOptions->verticalHeader( )->setSectionResizeMode( QHeaderView::ResizeToContents );
}

void dlg_config::change_clicked( )
{
	QWidget* pWidget = qobject_cast<QWidget*>( sender( )->parent( ) );

	if ( !pWidget )
		return;

	int row = ui->tblOptions->indexAt( pWidget->pos( ) ).row( );

	QTableWidgetItem* pItem = ui->tblOptions->item( row, 0 );

	if ( !pItem )
		return;

	dlg_key dk( pItem->data( Qt::UserRole ).toUInt( ) );
	dk.setModal( true );

	if ( dk.exec( ) != QDialog::Accepted )
		return;

	/*
	qDebug( ) << "[-] change:"
			  << pItem->text( )
			  << "from:"
			  << QString::number( pItem->data( Qt::UserRole ).toUInt( ), 16 )
			  << "to:"
			  << QString::number( dk.get_new_value( ), 16 );
	*/

	m_settings->setValue( pItem->text( ), "0x" + QString::number( dk.get_new_value( ), 16 ) );
	m_settings->sync( );

	//delete m_settings;

	//m_settings->sync( );

	parse_config_file( ui->cmbConfigFiles->currentText( ) );
}
