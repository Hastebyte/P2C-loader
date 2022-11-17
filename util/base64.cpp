#include "base64.h"

namespace util
{
    const std::string base64_encode( const std::string& plain_text )
    {
        DWORD size = 0;

        if ( !CryptBinaryToStringA( reinterpret_cast<const BYTE*>( plain_text.c_str( ) ), ( DWORD )plain_text.length( ),
                                    CRYPT_STRING_BASE64 | CRYPT_STRING_PERCENTESCAPE | CRYPT_STRING_NOCRLF, NULL, &size ) )
        {
            return { };
        }

        char* encoded_string
            = reinterpret_cast<char*>( HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, size + 1 ) );

        if ( !encoded_string )
        {
            return { };
        }

        if ( !CryptBinaryToStringA( reinterpret_cast<const BYTE*>( plain_text.c_str( ) ), ( DWORD )plain_text.length( ),
                                    CRYPT_STRING_BASE64 | CRYPT_STRING_PERCENTESCAPE | CRYPT_STRING_NOCRLF, encoded_string, &size ) )
        {
            HeapFree( GetProcessHeap( ), 0, encoded_string );
            return { };
        }

        encoded_string[size] = NULL;
        std::string result = encoded_string;

        HeapFree( GetProcessHeap( ), 0, encoded_string );
        return result;
    }

    const std::string base64_decode( const std::string& encoded_text )
    {
        DWORD size = 0;

        if ( !CryptStringToBinaryA( encoded_text.c_str( ), ( DWORD )encoded_text.length( ), CRYPT_STRING_BASE64 | CRYPT_STRING_PERCENTESCAPE, NULL, &size, NULL, NULL ) )
        {
            return { };
        }

        BYTE* decoded_string = reinterpret_cast<BYTE*>( HeapAlloc( GetProcessHeap( ), HEAP_ZERO_MEMORY, ( size + 1 ) * sizeof( BYTE ) ) );

        if ( !decoded_string )
        {
            return { };
        }

        if ( !CryptStringToBinaryA( encoded_text.c_str( ), ( DWORD )encoded_text.length( ), CRYPT_STRING_BASE64 | CRYPT_STRING_PERCENTESCAPE, decoded_string, &size, NULL, NULL ) )
        {
            HeapFree( GetProcessHeap( ), 0, decoded_string );
            return { };
        }

        decoded_string[size] = 0;
        std::string result = std::string( reinterpret_cast<char*>( decoded_string ) );

        HeapFree( GetProcessHeap( ), 0, decoded_string );
        return result;
    }


}
