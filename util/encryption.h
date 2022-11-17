#pragma once

#include <QString>
#include <QByteArray>
#include <QDebug>

#include "VMProtectSDK.h"

const unsigned char communication_key[] = { 0xD0, 0xF2, 0x01, 0x12, 0xEF, 0x22 };
const unsigned char packaged_binary_key[] = { 0xe2, 0x4e, 0x49, 0xd1, 0xc6, 0xcd, 0x9e, 0x8b, 0xba, 0x67 };

void encrypt_forward_simple( const std::string& in, std::string &out );
QString encrypt_encode( const QString& data );
QString decode_decrypt( const QString& data );
QByteArray decrypt( const QByteArray& buffer );
