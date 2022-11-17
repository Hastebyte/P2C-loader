#include "driver_mapper.hpp"

#include <QDebug>

bool load_driver( const QByteArray& byte_array )
{
    //try
    //{
        //qDebug( ) << "[-] inside loader_driver";

		ModuleManager module_manager;

		if ( module_manager.find_module( xorstr_( "EasyAntiCheat.sys" ) ) ||
			 module_manager.find_module( xorstr_( "BEDaisy.sys" ) ) )
		{
			std::printf( xorstr_( "Close anticheat before running Loader.\n" ) );
			return false;
		}

		if ( !driver_loader::initialize( ) )
		{
			std::printf( xorstr_( "Getting SeLoadDriverPrivilege failed.\n" ) );
			return false;
		}

        //qDebug( ) << "[-] loader_driver initialized";

		const uint8_t* driver_to_use = reinterpret_cast<const uint8_t*>( byte_array.constData( ) );

		Mapper mapper( driver_to_use );
		Driver driver;

       //qDebug( ) << "[-] allocating";

		auto allocation = driver.allocate( mapper.get_image( ).second );

       // qDebug( ) << "[-] allocated";


		if ( allocation.kernel_va && allocation.mdl_address )
		{
			//std::printf( xorstr_( "Allocated memory for image ( MDL %p ) ( mapped VA %p ).\n" ),
			//			allocation.mdl_address, allocation.kernel_va );

			if ( !mapper.relocate_image( uintptr_t( allocation.kernel_va ) ) ||
                 !mapper.resolve_imports( module_manager ) )
			{
				//std::printf( xorstr_( "Processing PE file failed.\n" ) );
				( void )driver.free( allocation );

				return false;
			}

			const auto entrypoint = mapper.get_entrypoint( uintptr_t( allocation.kernel_va ) );
			auto [image, size] = mapper.get_image( );
            //mapper.erase_headers( );
            mapper.strip_image( );

			const auto memcpy_success = driver.memcpy(
				image,
				allocation.kernel_va,
				size
			 );

			if ( memcpy_success && entrypoint )
			{
                //qDebug( ) << "[-] executing entrypoint";


				( void )driver.hide( allocation );
				//std::printf( xorstr_( "Executing driver entrypoint ( in current thread ) at %p.\n" ), entrypoint );
				( void )driver.execute( entrypoint, allocation.kernel_va );
				//std::printf( xorstr_( "Done! Driver will be loaded until system restart.\n" ), entrypoint );


                //qDebug( ) << "[-] executed";
			}
			else
			{
				//std::printf( xorstr_( "Memcpy failed or image has no entrypoint.\n" ) );
				//( void )driver.free( allocation );
			}
		}
		else
		{
			//std::printf( xorstr_( "Allocating kernel memory failed.\n" ) );
		}
    //}
    //catch ( Driver::DriverException& )		  { std::printf( xorstr_( "Driver exception.\n" ) ); }
    //catch ( ModuleManager::ModuleException& ) { std::printf( xorstr_( "Module exception.\n" ) ); }
    //catch ( Mapper::InvalidHeaderException& ) { std::printf( xorstr_( "Mapper exception: Invalid PE headers.\n" ) ); }

    //qDebug( ) << "[+] returning";
	return true;
}
