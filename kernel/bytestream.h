// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "bytearray.h"
#include "classutils.h"
#include "stringutils.h"

class ByteStream {
public:
    ByteStream(ByteArray * array, const char * terminators = nullptr) {
        m_pData = array;
        if (terminators) {
            m_terminators = std::string(terminators);
        }
    }

    ByteStream(const ByteArray & array, const char * terminators = nullptr) {
        m_pData = const_cast<ByteArray*>(&array);
        if (terminators) {
            m_terminators = std::string(terminators);
        }
    }

    ~ByteStream() = default;

    void    seek(int pos) { m_nPosition = pos; };
    void    rewind() { seek(0); };

    int     pos() const { return m_nPosition; };
    int     size() const { return (m_pData ? m_pData->size() : 0); };
    void    close() { m_pData = nullptr; m_nPosition = 0; };
    
    bool    isEmpty() const { return (m_pData ? m_pData->size() > 0 : false); };
    bool    atEnd() const { return (m_pData ? m_nPosition >= m_pData->size() : true); };

    std::string     fetchString(int maxLength) {
        std::string     result;
        if (!m_pData || maxLength <= 0)
            return result;

        while (!atEnd() && (*m_pData)[m_nPosition] && (result.size() < (size_t)maxLength)) {
            unsigned char ch = (*m_pData)[m_nPosition];

            if (m_terminators.size()) {
                if (m_terminators.find(ch) != std::string::npos) {
                    m_nPosition = m_pData->size();
                    break;
                }
            }

            result += (*m_pData)[m_nPosition];
            m_nPosition++;
        }

        return result;
    }

    int             fetchInt(int fieldLength, bool * bOk = nullptr ) { //-V1071
        if (bOk)
            *bOk = true;

        std::string result = fetchString(fieldLength);
        if (!result.size()) {
            if (bOk)
                *bOk = false;
            return 0;
        }

        return atoi(result.c_str());
    }

    bool           expect(const std::string & value, bool bIgnoreCase = true) {
        if (value.size() == 0)
            return true;

        std::string result      = fetchString(value.size());
        std::string compareTo   = value;
        if (bIgnoreCase) {
            StringUtils::toUpper(result);
            StringUtils::toUpper(compareTo);
        }

        if (result != compareTo) {            
            return false;
        }

        return true;
    }

    DISABLE_COPY_AND_MOVE(ByteStream);

protected:

    std::string         m_terminators;
    ByteArray   *       m_pData     = nullptr;
    int                 m_nPosition = 0;
};