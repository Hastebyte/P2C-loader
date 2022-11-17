#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QSslConfiguration>
#include <QTimer>
#include <QDebug>
#include <QThread>

// include this before anything else

#include "util/web_api.h"
#include <Windows.h>

#include "core/translation_manager.h"
#include "core/authenticator.h"

#include "core/driver_mapper/driver_mapper.hpp"
#include "core/kdmapper/intel_driver.hpp"
#include "core/kdmapper/kdmapper.hpp"
#include "core/dll_mapper/dll_mapper.hpp"
#include "core/dll_mapper2/usermode_mapper.h"
#include "core/dll_injector/injector.h"

#include "core/verifier/verifier.h"
#include "core/verifier/verifier2.h"

#include "dialogs/dlg_config.h"
#include "dialogs/dlg_hardware.h"

#include "util/kernel.h"
#include "util/process.h"
#include "util/shared.h"


#include "VMProtectSDK.h"
//#include "VirtualizerSDK.h"

#define LOADER_CHINA_BUILD 0

namespace Ui
{
	class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
	explicit MainWindow( QWidget* parent = nullptr );
	~MainWindow( );

public slots:
	void on_close_requested( );

private slots:
	void on_cmbLanguage_currentTextChanged( const QString& arg1 );
	void on_cmdAuthenticate_clicked( );
	void on_cmdQuit_clicked( );
	void on_cmbGame_currentIndexChanged( int index );
	void on_cmdOptions_clicked( );
	void on_cmdHardware_clicked( );

private:
	Ui::MainWindow* ui;
	translation_manager translations;
	QString m_status_definition;
	unsigned int m_status_code;
	bool m_block_auth;

private:
	void change_label_color( QLabel* label, QColor color );
	void update_status_label( QString label_definition, unsigned int code = 0 );
	language_definition* selected_language( const QString& language );
	void apply_translation( const QString& language );
    HANDLE launch_dummy( const char* path );
	void clean_traces( );
    bool launch_filter( );
    bool launch_cheat( authenticator* auth, QString cheat_file, unsigned int game_id );
    bool launch_exe_blocking( const QString& path, bool block );
    bool launch_cleaner( );
    bool load_emulator( );
	bool show_server_message( );
	bool check_secure_boot( );
	void save_previous_key( );
	void load_previous_key( );
	void clean_prefetch( );
};

#endif // MAINWINDOW_H
