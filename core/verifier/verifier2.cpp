#include "verifier2.h"

__declspec( noinline ) bool create_shared_section( const wchar_t* section_name, HANDLE& file_mapping, uint8_t*& file_view )
{
	VMProtectBeginMutation( "create_shared_section" );

	static NTSTATUS ( NTAPI* RtlAdjustPrivilege )( ULONG, BOOLEAN, BOOLEAN, PBOOLEAN );

	if ( !RtlAdjustPrivilege )
	{
		RtlAdjustPrivilege
				= reinterpret_cast<decltype( RtlAdjustPrivilege )>( GetProcAddress( GetModuleHandleW( xorstr_( L"ntdll") ), xorstr_( "RtlAdjustPrivilege" ) ) );
	}

	BOOLEAN enabled = false;

	if ( RtlAdjustPrivilege( 30, true, false, &enabled ) && !enabled )
		return false;

	file_mapping = CreateFileMapping( INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 0x1000, section_name );

	if ( !file_mapping )
		return false;

	file_view = reinterpret_cast<uint8_t*>( MapViewOfFile( file_mapping, FILE_MAP_ALL_ACCESS, 0, 0, 0x1000 ) );

	if ( !file_view )
	{
		CloseHandle( file_mapping );
		return false;
	}

	VMProtectEnd( );

	return true;
}

__declspec( noinline ) bool create_loader_challenge( unsigned int game_id )
{
	VMProtectBeginMutation( "verify_loader" );

	HANDLE file_mapping;
	uint8_t* file_view;

    if ( game_id == 8 || game_id == 13 || game_id == 99 || game_id == 100 )
    {
        // abstracts codes
        // qDebug( ) << "[-] special case rust challenge";

        if ( !create_shared_section( xorstr_( L"Global\\Ebx4vkivUEQF9yvuicxIrUEXpUXmRJBQ1z4K5" ), file_mapping, file_view ) )
            return false;

    } else if ( game_id == 15 ) {

        // girs codes

        if ( !create_shared_section( xorstr_( L"Global\\LplMU4kg5vq7JILyVCyQnSpipfSUosn1iUf" ), file_mapping, file_view ) )
            return false;


    } else if ( game_id == 7 ) {


        if ( !create_shared_section( xorstr_( L"Global\\yyuiG0yg976G0yG7uygh08YTGyugh067GT0gb06gf0" ), file_mapping, file_view ) )
            return false;

    } else {

        if ( !create_shared_section( xorstr_( L"Global\\TZ7VNqIF085rkxGrqK5Ufby4b9EWmCCIo2fj8Lh5" ), file_mapping, file_view ) )
            return false;

    }

	//qDebug( ) << "File mapping:" << file_mapping;
	//qDebug( ) << "File view:" << file_view;

	std::random_device rd;
	std::uniform_int_distribution<int> dist( 0,255 );

	// fill with random bytes

	for ( uint64_t i = 0; i < loader_data_offset; i++ )
		file_view[i] = static_cast<uint8_t>( dist( rd ) & 0xFF );

	LARGE_INTEGER counter { };
	QueryPerformanceCounter( &counter );

	loader_challenge buffer = { };
	buffer.magic1		= loader_magic_1;
	buffer.magic2		= loader_magic_2;
	buffer.game_id		= game_id;

	buffer.language_id	= 0;
    buffer.reseller_id  = 20;
	buffer.create_time	= counter.QuadPart ^ loader_time_encryption_key;
	buffer.checksum		= 0xFFFFFFFF;

	memcpy( file_view + loader_data_offset, &buffer, sizeof( loader_challenge ) );

	// fill rest with random bytes

	const uint64_t data_end = loader_data_offset + sizeof( loader_challenge );

	//qDebug( ) << "data end: 0x" << QString::number( data_end, 16 );

	for ( uint64_t i = data_end; i < 0x1000 - data_end; i++ )
		file_view[i] = static_cast<uint8_t>( dist( rd ) & 0xFF );

	UnmapViewOfFile( file_view );

	//qDebug( ) << "Client portion";

	VMProtectEnd( );

	return true;
}
