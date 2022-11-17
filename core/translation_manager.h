#pragma once

#include <QVector>
#include <QList>
#include <QPair>
#include <QString>
#include <QResource>
#include <QFile>
#include <QDebug>

class language_definition
{
public:
	QString name;
	QList<QPair<QString, QString>> strings;

	~language_definition( ) { strings.clear( ); }
	 language_definition( ) { }

	 QString get( QString definition );
};

class translation_manager
{
private:
	QVector<language_definition*> languages;

public:

   ~translation_manager( ) { languages.clear( ); }
	translation_manager( ) { }

	bool add_language( QString name, QString resource_path );
	int language_count( );
	language_definition* get_language( QString name );
};
