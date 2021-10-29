// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <string>
#include <cctype>
#include <functional>
#include <memory>
#include <sstream>

namespace StringUtils {

    bool                    endsWith(std::string const &str, std::string const &ending);
    
    std::string             toUpper(const std::string & orig);
    void                    toUpper(std::string & s);

    std::string             toLower(const std::string & orig);
    void                    toLower(std::string & s);

    std::vector<std::string>            split(const std::string & str, char delimeter, bool skipEmpty = true, std::function <bool(const std::string & item)> filter = nullptr);
    std::pair<std::string, std::string> splitPair(const std::string & str, char delimeter);

    template<typename T> std::string  join(const T & container, std::function<std::string(typename T::value_type const & t)> f, const std::string & separator = ",") {
        std::string result;
        bool bFirst = true;
        for (auto item : container) {
            if (!bFirst) {
                result += separator;
            }
            result += f(item);
            bFirst = false;
        }

        return result;
    }

    template<typename T> std::vector<T> split(const char * str, char delimeter, std::function <T(const std::string & item)> f = nullptr, bool skipEmpty = true, std::function<bool(const std::string & item)> filter = nullptr) {
        std::vector<T>  items;
        std::stringstream ss(str);
        std::string item;

        while (getline(ss, item, delimeter)) {
            if (skipEmpty && item.size() == 0)
                continue;

            if (filter && !filter(item)) 
                continue;
            
            items.emplace_back(f(item));
        }

        return items;
    }


    template<typename T> std::string  joinStrings(const T & container, char separator = ',') {
        std::string result;
        bool bFirst = true;


        for (auto item : container) {
            if (!bFirst) {
                result += separator;
            }

            result += item;

            bFirst = false;
        }

        return result;
    }

    template<typename T> std::string  join(const T & container, char separator = ',') {
        std::string result;
        bool bFirst = true;

      
        for (auto item : container) {
            if (!bFirst) {
                result += separator;
            }

            result += std::string(std::to_string(item));
           
            bFirst = false;
        }

        return result;
    }

    template<typename ... Args> std::string format( const char * format, Args ... args ) {
	    int size_s = snprintf( nullptr, 0, format, args ... ) + 1; 
	    if( size_s <= 0 ) 
            return std::string();

        size_t size = static_cast<size_t>( size_s );
        auto buf = std::make_unique<char[]>( size );
        snprintf( buf.get(), size, format, args ... );
        return std::string( buf.get(), buf.get() + size - 1 );
	}
};