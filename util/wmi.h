#pragma once

#include <QString>
#include <QDebug>

#include <Windows.h>
#include <WbemIdl.h>
#include <comdef.h>

#include "xorstr.h"
#include "VMProtectSDK.h"

bool wmi_connect( IWbemLocator* &pLocator, IWbemServices* &pServices );
bool wmi_execute( IWbemServices* pServices, const wchar_t* query, IEnumWbemClassObject* &pEnumerator );
bool wmi_get_next_result( IEnumWbemClassObject* pEnumerator, IWbemClassObject* &pClassObject );
wchar_t* wmi_get_result_wide_string( IWbemClassObject* pClassObject, _variant_t& vt, const wchar_t* field );
long wmi_get_result_long( IWbemClassObject* pClassObject, _variant_t& vt, const wchar_t* field );
uint64_t wmi_get_result_uint64( IWbemClassObject* pClassObject, _variant_t& vt, const wchar_t* field );
