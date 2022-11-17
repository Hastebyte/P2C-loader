#include "wmi.h"

bool wmi_connect( IWbemLocator* &pLocator, IWbemServices* &pServices )
{
	VMProtectBeginMutation( "wmi_connect" );

	pLocator = nullptr;
	HRESULT hrCreateInstance = CoCreateInstance( CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<void**>( &pLocator ) );

	if ( FAILED( hrCreateInstance ) )
	{
		qDebug( ) << "[!] CoCreateInstance failed: " << QString::number( hrCreateInstance, 16 );
		return false;
	}

	pServices = nullptr;
	HRESULT hrConnectServer = pLocator->ConnectServer( _bstr_t( xorstr_( "\\\\.\\root\\cimv2" ) ), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pServices );

	if ( FAILED( hrConnectServer ) )
	{
		qDebug( ) << "[!] ConnectServer failed";
		pLocator->Release( );
		return false;
	}

	HRESULT hrSetProxy
			= CoSetProxyBlanket( pServices,
								 RPC_C_AUTHN_DEFAULT,
								 RPC_C_AUTHZ_NONE,
								 COLE_DEFAULT_PRINCIPAL,
								 RPC_C_AUTHN_LEVEL_DEFAULT,
								 RPC_C_IMP_LEVEL_IMPERSONATE,
								 nullptr,
								 EOAC_NONE );

	if ( FAILED( hrSetProxy ) )
	{
		qDebug( ) << "[!] CoSetProxyBlanket failed";
		pLocator->Release( );
		pServices->Release( );
		return false;
	}

	VMProtectEnd( );
	return true;
}

bool wmi_execute( IWbemServices* pServices, const wchar_t* query, IEnumWbemClassObject* &pEnumerator )
{
	VMProtectBeginMutation( "wmi_execute" );

	BSTR bstr_query	= SysAllocString( query );
	BSTR bstr_language = SysAllocString( L"WQL" );

	pEnumerator = nullptr;

	HRESULT hrExecQuery
			= pServices->ExecQuery( bstr_language, bstr_query, WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, nullptr, &pEnumerator );

	if ( FAILED( hrExecQuery ) )
		return false;

	VMProtectEnd( );
	return true;
}


bool wmi_get_next_result( IEnumWbemClassObject* pEnumerator, IWbemClassObject* &pClassObject )
{
	VMProtectBeginMutation( "wmi_get_next_result" );

	pClassObject = nullptr;

	ULONG objects_returned = 0;
	HRESULT hrNext = pEnumerator->Next( WBEM_INFINITE, 1, &pClassObject, &objects_returned );

	if ( FAILED( hrNext ) || !objects_returned )
		return false;

	VMProtectEnd( );
	return true;
}

wchar_t* wmi_get_result_wide_string( IWbemClassObject* pClassObject, _variant_t& vt, const wchar_t* field )
{
	pClassObject->Get( field, NULL, &vt, nullptr, nullptr );
	return reinterpret_cast<wchar_t*>( vt.bstrVal );
}

long wmi_get_result_long( IWbemClassObject* pClassObject, _variant_t& vt, const wchar_t* field )
{
	pClassObject->Get( field, NULL, &vt, nullptr, nullptr );
	return vt.GetVARIANT( ).lVal;
}

uint64_t wmi_get_result_uint64( IWbemClassObject* pClassObject, _variant_t& vt, const wchar_t* field )
{
	pClassObject->Get( field, NULL, &vt, nullptr, nullptr );
	return vt.GetVARIANT( ).ullVal;
}
