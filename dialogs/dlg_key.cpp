#include "dlg_key.h"
#include "ui_dlg_key.h"

dlg_key::dlg_key( unsigned int key, QWidget* parent ) : QDialog( parent ), ui( new Ui::dlg_key )
{
	ui->setupUi( this );
	this->m_curent_key = key;

	ui->txtKeyName->setText( get_key_name( key ) );
	ui->txtNumericalValue->setText( "0x" + QString::number( key, 16 ) );
}

dlg_key::~dlg_key( )
{
	delete ui;
}

unsigned int dlg_key::get_new_value( )
{
	return this->m_new_key;
}

void dlg_key::update_text( )
{
	ui->txtNumericalValue->setText( "0x" + QString::number( m_new_key, 16 ) );
	ui->txtKeyName->setText( get_key_name( m_new_key ) );
}

void dlg_key::on_cmdSave_clicked( )
{
	accept( );
}

void dlg_key::on_cmdCancel_clicked()
{
	reject( );
}

bool dlg_key::eventFilter( QObject* obj, QEvent* event )
{
	Q_UNUSED( obj );

	if ( event->type( ) == QEvent::KeyPress )
	{
		QKeyEvent* key_event = reinterpret_cast<QKeyEvent*>( event );
		m_new_key = key_event->nativeVirtualKey( );

		removeEventFilter( this );
		update_text( );
	}

	if ( event->type( ) == QEvent::MouseButtonPress )
	{
		QMouseEvent* mouse_event = reinterpret_cast<QMouseEvent*>( event );
		m_new_key = mouse_event->button( );

		removeEventFilter( this );
		update_text( );

		/*
			switch ( mouse_event->button( ) )
			{
			case Qt::LeftButton;
				break;
			case Qt::RightButton:
				break;
			case Qt::MiddleButton:
				break;
			default:
				break;
			}
		*/
	}

	return false;
}

void dlg_key::on_cmdPress_clicked( )
{
	this->installEventFilter( this );
}
