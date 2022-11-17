//#endif
//#ifndef DLG_CONFIG_H
//#define DLG_CONFIG_H

#pragma once

#include <QDialog>
#include <QString>
#include <QPushButton>
#include <QSettings>
#include <QDir>
#include <QDebug>

#include <Windows.h>

#include "../core/translation_manager.h"

namespace Ui
{
	class dlg_config;
}

class dlg_config : public QDialog
{
Q_OBJECT

public:
	explicit dlg_config( QWidget* parent = nullptr );
	~dlg_config( );

private slots:
	void on_cmbConfigFiles_currentIndexChanged( const QString& text );
	void change_clicked( );

private:
	Ui::dlg_config* ui;
	QSettings* m_settings;

private:
	void create_header( );
	void populate_config_list( );
	void parse_config_file( const QString& file_name );
};

//#endif // DLG_CONFIG_H
