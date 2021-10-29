#pragma once

#include "bytearray.h"

#include <string>

namespace ByteArrayUtils {

    ByteArray       zip(const ByteArray & data);
    ByteArray       unzip(const ByteArray & data);    
    ByteArray       fromString(const char * data, char delimeter = ',');
    std::string     toString(const ByteArray & data, char delimeter = ',');
};

