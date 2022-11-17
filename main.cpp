#include "MainWindow.h"
#include <QApplication>
#include <QStyleFactory>

#include <QScreen>
#include <QDebug>

#include <intrin.h>

#include "util/shared.h"

/*
void hwid( )
{
	uint32_t regs[4];
	__cpuidex( (int *)regs, 0, 0 );

	printf( "cpuid: %X %X %X %X\n", regs[0], regs[1], regs[2], regs[3] );
	// cpuid: D 756E6547 6C65746E 49656E69
}

// clear cache
// https://github.com/VFPX/Win32API/blob/master/samples/sample_350.md

void get_cpu_hardware_id( )
{
	DWORD length;
	GetLogicalProcessorInformation( nullptr, &length );

	qDebug( ) << "Size required:" << length;

	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>( malloc( length ) );

	if ( !buffer )
		return;

	BOOL status = GetLogicalProcessorInformation( buffer, &length );

	if ( !status )
	{
		qDebug( ) << "[!] Failed to get processor information:" << GetLastError( );
	}

	qDebug( ) << "[+] flags:" << buffer->ProcessorCore.Flags;

}

#include "core/driver_mapper/driver_binary.hpp"
#include <fstream>
*/


int main( int argc, char* argv[] )
{
	QApplication a( argc, argv );
	a.setStyle( QStyleFactory::create( "Fusion" ) );


	QFileInfo info( QCoreApplication::applicationFilePath( ) );

	//qDebug( ) << info.dir( ).path( );
	//qDebug( ) << info.fileName( );

	QFile::rename( QCoreApplication::applicationFilePath( ),
                   info.dir( ).path( ) + "/" + random_string( 17 ) + ".exe" );

	//qDebug( ) << "encrypted server verifier.vip:" << encrypt_encode( "verifier.vip" );
	//qDebug( ) << "encrypted server identityservices.xyz:" << encrypt_encode( "identityservices.xyz" ); //
	//qDebug( ) << "encrypted server identityservices.pw:" << encrypt_encode( "identityservices.pw" ); //
	//qDebug( ) << "decrypted server:" << decode_decrypt( "uZZkfJtLpItyd51UuZFkYcFSpw==" ); // 167.99.170.199
    //qDebug( ) << "debug:" <<
    //             decode_decrypt( "ib9PfaVgooJjaKF1o6hyeLpKnItNINwR4sRFRoNDt5lWJqVll5M0QJNxh8cxSLhVv6doeYhz4st4SLxKhaZSeYhDhJF1XKtB5b9EYYhz4bBXW6pjt7xCJphvlZZId4FV5JRFQNdviIUxbthe4cc5ItoV5sA0Jg==" );


    qDebug( ) << "debug:" << set_debug_privilege( );
/*
    SHELLEXECUTEINFO ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = L"runas";
    ShExecInfo.lpFile = L"powershell";
    ShExecInfo.lpParameters = L"Start-Process C:\\Windows\\System32\\notepad.exe -Verb RunAs";
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_SHOW;
    ShExecInfo.hInstApp = NULL;
    ShellExecuteEx(&ShExecInfo);
    WaitForSingleObject(ShExecInfo.hProcess,INFINITE);
*/


	//QString cpu_string = hwid_cpu_name( ) + "|" + hwid_cpu_info( );
	//qDebug( ) << "cpu info" << cpu_string;

	//hwid( );
	//get_cpu_hardware_id( );

    //QDateTime current( QDateTime::currentDateTime( ) );
    //uint unix_time = current.toTime_t( );

    //qDebug( ) << "unixtime:" << unix_time;

	MainWindow w;
	w.show( );


	return a.exec( );
}
