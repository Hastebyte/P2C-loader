#pragma once

#include "../util/web_api.h"

#include <QVector>
#include <QString>
#include <QStringList>
#include <QDebug>
#include <QObject>
#include <ShlObj.h>

#include <QDir>
#include <QFile>
#include <QDirIterator>
#include <QMessageBox>
#include <QStandardPaths>

#include <QDateTime>

#include <QEventLoop>
#include <QHostAddress>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QCoreApplication>

#include "../util/encryption.h"
#include "../util/hwid.h"
#include "../util/xorstr.h"
#include "../util/shared.h"
#include "../util/wmi.h"
#include "../util/base64.h"

#include "VMProtectSDK.h"
//#include "VirtualizerSDK.h"

#pragma comment( lib, "shell32.lib" )

constexpr unsigned int loader_version = 221;

enum E_GAME_ID : unsigned int
{
    game_siege          = 0,
    game_dayz           = 1,
    game_rust           = 2,
    game_scum           = 3,
    game_apex           = 4,
	game_fortnite		= 5,
    game_pubg           = 6,
    game_siege2         = 7,
    game_rust2          = 8,
    game_fifa           = 9,
	game_overwatch		= 10,
    game_apex2          = 11,
	game_pubglite		= 12,
    game_tarkov         = 13,
    game_arma           = 14,
    game_gta5           = 15,
	hardware_spoof2		= 98,
	hardware_spoof		= 99,
    game_test           = 100,
};

enum E_CLEAN_LEVEL : unsigned int
{
    cleaner_disabled    = 0,
    cleaner_regular     = 1,
    cleaner_max         = 2,
};


enum E_SERVER_RESPONSE_CODE : unsigned int
{
	server_success					= 402,
	server_success_first_registration		= 403,
	server_failure_invalid_serial			= 404,
	server_failure_invalid_computerid		= 405,
	server_failure_expired_serial			= 406,
	server_failure_banned				= 407,
	server_failure_database				= 408,
	server_failure_outdated				= 409,
	server_failure_bad_request			= 410,
	server_failure_maintenance			= 411,
};

enum E_UI_RESPONSE_CODE : unsigned int
{
	ui_success								= 0,

	ui_failure_cache_validation				= 1, // serial not found in cache list
	ui_failure_invalid_load_balancer		= 2, // invalid ip returned from cache
	ui_failure_invalid_server_response		= 3, // nothing returned
	ui_failure_invalid_server_response2		= 4, // invalid data returned (one user had malware inject javascript into packets)
	ui_failure_invalid_loader_version		= 5, // loader is out of date
	ui_failure_invalid_server_response3		= 6, // unknown server response code

	ui_failure_invalid_serial				= 7,
	ui_failure_invalid_computerid			= 8,
	ui_failure_expired_serial				= 9,
	ui_failure_maintenance					= 10,

	ui_failure_program_path					= 11, // failed to get program files env variable
	ui_failure_download_cheat				= 12, //
	ui_failure_failed_to_write_file			= 13, // file to write out file
	ui_failure_download_driver				= 14, //

	ui_failure_mapper_exception				= 15, // mapper failed to verify & load driver


	ui_failure_blacklisted_application		= 20,
};

class authenticator : public QObject
{
	Q_OBJECT

public:
	QString m_serial_key;
	unsigned int m_game_id;

	QString m_cheat_url;
	QString m_driver_url;
    QByteArray m_cheat_byte_array;
	QByteArray m_driver_byte_array;
	QByteArray m_decrypted_driver_byte_array;
	QString m_cheat_file;

private:
	QString pick_key_cache_server( );
	QString pick_ip_cache_server( );

	QByteArray get_url_native( const QString& url );
	QByteArray post_url_native( const QString& url, const QString& resource, const QString& data );
	QByteArray get_url( const QString& url );
	QByteArray post_url( const QString& url, const QString& post_data );

	bool validate_ip( const QString& address );
	QString get_auth_server( );

public:

   ~authenticator( ) { }

	authenticator( QString key, unsigned int game_id, QObject* parent = nullptr ) : QObject( parent )
	{
		m_serial_key = key.trimmed( );
		m_game_id = game_id;
	}

	QString get_monitor_serial( );
	bool validate_key_format( );
	bool validate_key_against_cache( );
	unsigned int validate_key( );
	bool clean_old_files( const QString& path );
    //int progress_func( void* ptr, double total_to_download, double downloaded, double total_to_upload, double uploaded );
    //bool download_http_file( QString url, memory_struct& buffer );
	unsigned int download_files( );
	QByteArray driver_byte_array( );
	QString cheat_file( );
};
