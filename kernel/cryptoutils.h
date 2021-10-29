// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

class CryptoUtils {
public:
	static unsigned short crc16(const unsigned char * pcBlock, unsigned short len);
};
