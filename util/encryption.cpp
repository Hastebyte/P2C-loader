#include "encryption.h"

void encrypt_forward_simple( const std::string& in, std::string &out )
{
    out = in;

    for ( int i = 0; i < in.size( ); i++ )
        out[i] = in[i] ^ communication_key[i % ( sizeof( communication_key ) / sizeof( char ) )];
}

__declspec( noinline ) QString encrypt_encode( const QString& data )
{
	VMProtectBeginMutation( "encrypt_encode" );

	if ( data.length( ) == 0 )
		return QString( );

	QByteArray in_array = data.toLocal8Bit( );

	QByteArray result;

	for ( int i = 0; i < data.size( ); i++ )
		result.append( in_array[i] ^ communication_key[i % ( sizeof( communication_key ) / sizeof( unsigned char ) )] );

	VMProtectEnd( );

	return QString::fromLocal8Bit( result.toBase64( ) );
}

__declspec( noinline ) QString decode_decrypt( const QString& data )
{
	VMProtectBeginMutation( "decode_decrypt" );

	if ( data.length( ) == 0 )
		return QString( );

	QByteArray in_array = QByteArray::fromBase64( data.toLocal8Bit( ) );

	QByteArray result;

	for ( int i = 0; i < in_array.size( ); i++ )
		result.append( in_array[i] ^ communication_key[i % ( sizeof( communication_key ) / sizeof( unsigned char ) )] );

	VMProtectEnd( );

	return QString( result );
}

__declspec( noinline ) QByteArray decrypt( const QByteArray& buffer )
{
	VMProtectBeginMutation( "decrypt" );

	QByteArray result;

	if ( buffer.length( ) == 0 )
		return result;

	for ( int i = 0; i < buffer.length( ); i++ )
		result.append( buffer[i] ^ communication_key[i % ( sizeof( communication_key ) / sizeof( unsigned char ) )] );

	VMProtectEnd( );

	return result;
}
