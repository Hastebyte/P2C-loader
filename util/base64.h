#ifndef BASE64_H
#define BASE64_H

#include "windows.h"
#include <stdio.h>
#include <time.h>
#include <string>

namespace util
{
    const std::string base64_encode( const std::string& plain_text );
    const std::string base64_decode( const std::string& encoded_text );
}


#endif
