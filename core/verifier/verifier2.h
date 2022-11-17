#include <Windows.h>
#include <iostream>
#include <vector>
#include <random>

#include "../../util/encryption.h"
#include "../../util/hwid.h"
#include "../../util/xorstr.h"
#include "../../util/shared.h"

#pragma pack( push, 1 )
struct loader_challenge
{
	uint64_t	magic1;						// check 1
	uint32_t	game_id;					//
	uint32_t	language_id;				//
	int64_t		create_time;				// check 3
	uint64_t	magic2;						// check 2
	uint64_t	random;						//
	uint8_t		encrypted_ini_path[256];	//
	uint64_t	mac_hash;					// not used yet
	uint64_t	serial_hash;				// not used yet
	uint64_t	checksum;					// not used yet
	uint32_t	reseller_id;
};
#pragma pack( pop )

constexpr uint64_t loader_data_offset			= 0x117;
constexpr uint64_t loader_magic_1				= 0x12398DFF8123A77C;
constexpr uint64_t loader_magic_2				= 0xADD7123FCC163203;
constexpr uint64_t loader_time_encryption_key	= 0x24922073969;

__declspec( noinline ) bool create_shared_section( const wchar_t* section_name, HANDLE& file_mapping, uint8_t*& file_view );
__declspec( noinline ) bool create_loader_challenge( unsigned int game_id );
