#ifndef dlg_hardware_H
#define dlg_hardware_H

#include <QDebug>
#include <QDialog>
#include <QVector>
#include <QString>
#include <QMessageBox>
#include <QFileInfo>
#include <QSettings>

#include <Windows.h>
#include <stdio.h>
#include <winternl.h>

#include "..\util\wmi.h"
#include "..\util\shared.h"

#pragma comment( lib, "ntdll.lib" )

#include "VMProtectSDK.h"

enum hwid_type : uint32_t
{
	E_HWID_UNKNOWN	= 0,
	E_HWID_NIC		= 1,
	E_HWID_DISK		= 2,
	E_HWID_SMBIOS	= 3,
	E_HWID_REGISTRY	= 4,
	E_HWID_FILES	= 5,
	E_HWID_USN		= 6,
	U_HWID_PREFETCH	= 7,
};


enum E_COMMAND_CODE_HWID : uint32_t
{
	ID_HWID_NULL				= 0,
	ID_NDIS_GET_DISPATCH		= 1,
	ID_NDIS_GET_HANDLER			= 2,
	ID_NDIS_HOOK_DISPATCH		= 3,
	ID_NDIS_GET_ADDRESS			= 4,
	ID_NDIS_SET_ADDRESS			= 5,

	ID_DISK_GET_DEVICE			= 6,
	ID_DISK_GET_SERIAL			= 7,
	ID_DISK_SET_SERIAL			= 8,
	ID_DISK_REGISTER_RAID		= 9,

	ID_SMART_GET_ID				= 10,
	ID_SMART_DISABLE_ID			= 11,

	ID_SMBIOS_GET_SIZE			= 12,
	ID_SMBIOS_READ				= 13,
	ID_SMBIOS_CLEAR				= 14,
};

//
// IDENTIFY data (from ATAPI driver source)
//

#pragma pack(1)

typedef struct _IDENTIFY_DATA {
    USHORT GeneralConfiguration;            // 00 00
    USHORT NumberOfCylinders;               // 02  1
    USHORT Reserved1;                       // 04  2
    USHORT NumberOfHeads;                   // 06  3
    USHORT UnformattedBytesPerTrack;        // 08  4
    USHORT UnformattedBytesPerSector;       // 0A  5
    USHORT SectorsPerTrack;                 // 0C  6
    USHORT VendorUnique1[3];                // 0E  7-9
    USHORT SerialNumber[10];                // 14  10-19
    USHORT BufferType;                      // 28  20
    USHORT BufferSectorSize;                // 2A  21
    USHORT NumberOfEccBytes;                // 2C  22
    USHORT FirmwareRevision[4];             // 2E  23-26
    USHORT ModelNumber[20];                 // 36  27-46
    UCHAR  MaximumBlockTransfer;            // 5E  47
    UCHAR  VendorUnique2;                   // 5F
    USHORT DoubleWordIo;                    // 60  48
    USHORT Capabilities;                    // 62  49
    USHORT Reserved2;                       // 64  50
    UCHAR  VendorUnique3;                   // 66  51
    UCHAR  PioCycleTimingMode;              // 67
    UCHAR  VendorUnique4;                   // 68  52
    UCHAR  DmaCycleTimingMode;              // 69
    USHORT TranslationFieldsValid:1;        // 6A  53
    USHORT Reserved3:15;
    USHORT NumberOfCurrentCylinders;        // 6C  54
    USHORT NumberOfCurrentHeads;            // 6E  55
    USHORT CurrentSectorsPerTrack;          // 70  56
    ULONG  CurrentSectorCapacity;           // 72  57-58
    USHORT CurrentMultiSectorSetting;       //     59
    ULONG  UserAddressableSectors;          //     60-61
    USHORT SingleWordDMASupport : 8;        //     62
    USHORT SingleWordDMAActive : 8;
    USHORT MultiWordDMASupport : 8;         //     63
    USHORT MultiWordDMAActive : 8;
    USHORT AdvancedPIOModes : 8;            //     64
    USHORT Reserved4 : 8;
    USHORT MinimumMWXferCycleTime;          //     65
    USHORT RecommendedMWXferCycleTime;      //     66
    USHORT MinimumPIOCycleTime;             //     67
    USHORT MinimumPIOCycleTimeIORDY;        //     68
    USHORT Reserved5[2];                    //     69-70
    USHORT ReleaseTimeOverlapped;           //     71
    USHORT ReleaseTimeServiceCommand;       //     72
    USHORT MajorRevision;                   //     73
    USHORT MinorRevision;                   //     74
    USHORT Reserved6[50];                   //     75-126
    USHORT SpecialFunctionsEnabled;         //     127
    USHORT Reserved7[128];                  //     128-255
} IDENTIFY_DATA, *PIDENTIFY_DATA;
#pragma pack()

#define STATUS_SUCCESS 0

class hw_item
{
public:
	hwid_type type;
	bool selected;

	 hw_item( hwid_type type ) : type( type ) { }
	~hw_item( ) { }
};

class nic_info : public hw_item
{
public:
	QString name;
	QString address;

	 nic_info( ) : hw_item( E_HWID_NIC ) { }
	~nic_info( ) { }
};

class disk_info : public hw_item
{
public:
	int index;
	QString device_id;
	QString model;
	QString serial;
	qulonglong size;

	bool operator< ( const disk_info& other ) const
	{
		qDebug( ) << "comparing:" << this->index << other.index;
		return this->index < other.index;
	}


	 disk_info( ) : hw_item( E_HWID_DISK ) { }
	~disk_info( ) { }
};


template <typename T>
struct ForwardLessThen
{
	bool operator( )( const T* a, const T* b ) const
	{
		return *a < *b;
	}
};

namespace Ui
{
	class dlg_hardware;
}

class dlg_hardware : public QDialog
{
Q_OBJECT

public:
	explicit dlg_hardware( QWidget* pParent = nullptr );
	~dlg_hardware( );

    QString get_smartctl( unsigned int drive_number );
    void get_mac( );
	bool get_system_disk_information( );
	bool populate_tree( );


	template<typename ... A>
	__declspec( noinline ) uint64_t call_driver_control( const A ... arguments )
	{
		VMProtectBeginMutation( "call_driver_control" );

		HMODULE hModule = LoadLibrary( xorstr_( L"win32u.dll" ) );

		if ( !hModule )
			return 0;

		void* function = reinterpret_cast<void*>( GetProcAddress( hModule, xorstr_( "NtQueryCompositionSurfaceHDRMetaData" ) ) );

		if ( !function )
			return 0;

		using tFunction = uint64_t( __stdcall* )( A... );
		const auto control = static_cast<tFunction>( function );

		VMProtectEnd( );

		return control( arguments ... );
	}

private:
    QString m_serial_disk0;
    QString m_serial_nic0;
	Ui::dlg_hardware* ui;
	QVector<disk_info*> m_disk;
};

#endif // dlg_hardware_H
