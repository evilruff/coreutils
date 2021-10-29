// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "bytearrayutils.h"

#include "stringutils.h"
#include "zlib.h"

namespace ByteArrayUtils {

    ByteArray       zip(const ByteArray & data) {
        unsigned long    compressedDataSize = data.size() + 10;
        unsigned char *  compressedData = (unsigned char *)calloc(1, compressedDataSize);

        if (compressedData == nullptr)
            return ByteArray();

        compress(compressedData, &compressedDataSize, data.data(), data.size());

        ByteArray result((const char*)compressedData, compressedDataSize);

        free(compressedData);

        return result;
    }

    ByteArray       unzip(const ByteArray & data) {
        unsigned long    uncompressedDataSize = data.size() * 50;
        unsigned char *  uncompressedData = (unsigned char *)calloc(1, uncompressedDataSize);

        if (uncompressedData == nullptr)
            return ByteArray();

        uncompress(uncompressedData, &uncompressedDataSize, (unsigned char*)data.data(), data.size());

        ByteArray result((const char*)uncompressedData, uncompressedDataSize);

        free(uncompressedData);

        return result;
    }

    ByteArray       fromString(const char * data, char delimeter) {
        return StringUtils::split<unsigned char>(data, delimeter, [](const std::string & item) { return (unsigned char)atoi(item.c_str()); });
    }

    std::string     toString(const ByteArray & data, char delimeter) {
        std::string result;
        std::vector<std::string>    items;
        int nSize = data.size();
        
        for (int i = 0; i < nSize; i++) {
            items.emplace_back(std::to_string(data[i]));
        }

        return StringUtils::joinStrings(items, delimeter);
    }
};


