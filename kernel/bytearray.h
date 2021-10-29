// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "shareddata.h"

#include <string.h>

#include <vector>
#include <string>

//---------------------------------------------------------------------
class   ByteArrayData: public SharedData {
public:
    ByteArrayData()  = default;
    ~ByteArrayData() = default;

    ByteArrayData(const ByteArrayData &other)
        : SharedData(other), data(other.data) { }
    
    std::vector<unsigned char>  data;
};
//---------------------------------------------------------------------
class   ByteArray
{
public:
    
    SHARED_DATA_CLASS_DECLARE(ByteArray, ByteArrayData, d);
    ByteArray(int n, unsigned char v = 0) {
        d = new ByteArrayData;
        std::vector<unsigned char>  tempValue(n,v);
        d->data = tempValue;
    }

    ByteArray(const std::vector<unsigned char> & data) {
        d = new ByteArrayData;
        d->data = data;
    }

    ByteArray(const char * ptr, int nSize = 0) {
        d = new ByteArrayData;
        
        if (nSize < 0)
            return;

        if (!nSize) {
            if (ptr) {
                nSize = strlen((const char*)ptr);
            }
            else {
                return;
            }            
        }

        std::vector<unsigned char>  tempValue(ptr, ptr + nSize);
        d->data = tempValue;
    }

    ByteArray &     operator +=(const ByteArray & other) {
        d->data.insert(d->data.end(), other.d->data.begin(), other.d->data.end());
        return *this;
    }

    ByteArray &     operator +=(unsigned char ch) {
        d->data.push_back(ch);
        return *this;
    }

    void    append(unsigned char ch) {
        d->data.push_back(ch);
    }

    std::string toHex(bool bLowerCase = false) const {
        static const char* digits      = "0123456789ABCDEF";
        static const char* lowerDigits = "0123456789abcdef";

        if (!d->data.size())
            return std::string();
     
        int i = 0;
        std::string outputString( d->data.size()*2, '0');

        for (unsigned char ch : d->data) {
            outputString[i] = bLowerCase ? lowerDigits[ch/16] : digits[ch / 16];
            outputString[i+1] = bLowerCase ? lowerDigits[ch % 16] : digits[ch % 16];
            i += 2;
        }

        return outputString;
    }

    void	resize(int n) { d->data.resize(n); };
    int		size() const { return d->data.size(); };
    bool	isEmpty() const { return (d->data.size() == 0) ? true : false; };
    const unsigned char * data() const { return d->data.data(); };
    std::string           toString() const { return std::string((const char*)d->data.data(), d->data.size()); };
	void    clear() { d->data.clear(); };

	void	remove(unsigned int from, int s = -1) {
		if (from >= d->data.size()) //-V807
			return;
		
		s = ((s == -1) || (from + s >= d->data.size())) ? d->data.size() - from : s;

		d->data.erase(d->data.begin() + from, d->data.begin()+from+s);
	}

    unsigned char &     operator[](int index) { return d->data[index]; };
    unsigned char       operator[](int index) const { return d->data[index]; };    
};
//---------------------------------------------------------------------
