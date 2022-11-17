#include "translation_manager.h"

QString language_definition::get( QString definition )
{
	QPair<QString, QString> pair;

	foreach( pair, this->strings )
	{
		if ( pair.first == definition )
			return pair.second;
	}

	return QString( );
}

bool translation_manager::add_language( QString name, QString resource_path )
{
	QResource resource( resource_path );
	QFile file( resource.absoluteFilePath( ) );

	if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
		return false;

	language_definition* lang = new language_definition( );
	lang->name = name;

	QTextStream in( &file );
	in.setCodec( "UTF-8" );

	while ( !in.atEnd( ) )
	{
		QString line = in.readLine( );
		QStringList parts = line.split( "=" );

		if ( parts.size( ) != 2 )
			continue;

		lang->strings.push_back( qMakePair( parts.at( 0 ), parts.at( 1 ) ) );
	}

	this->languages.push_back( lang );
	file.close( );

	return true;
}

int translation_manager::language_count( )
{
	return this->languages.count( );
}

language_definition* translation_manager::get_language( QString name )
{
	for ( language_definition* language : this->languages )
	{
		if ( name == language->name )
			return language;
	}

	return nullptr;
}
