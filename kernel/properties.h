// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <map>
#include <string>

#include "stringutils.h"

class Properties: public std::map<std::string, std::string> {
public:
    Properties() :std::map<std::string, std::string>() {};
    Properties(const Properties & other) : std::map<std::string, std::string>(other) {};
    Properties(const Properties && other) : std::map<std::string, std::string>(other) {};
    

    Properties & operator = (const Properties & other) { return *this; };
    Properties & operator = (const Properties && other)  { return *this; };

    void    setInt(const char * name, int v) {
        (*this)[StringUtils::toLower(name)] = std::to_string(v);
    }

    void    setDouble(const char * name, double v) {
        (*this)[StringUtils::toLower(name)] = std::to_string(v);
    }

    void    setString(const char * name, const char * v) {
        (*this)[StringUtils::toLower(name)] = std::string(v);
    }

    void    setString(const char * name, const std::string & v) {
        (*this)[StringUtils::toLower(name)] = v;
    }

    int     getInt(const char * name) const {
        if (contains(name)) {
            return atoi(at(StringUtils::toLower(name)).c_str());
        }

        return 0;
    }

    double         getDouble(const char * name) const {
        if (contains(name)) {
            return atof(at(StringUtils::toLower(name)).c_str());
        }

        return 0;
    }

    std::string     getString(const char * name) const {
        if (contains(name)) {
            return at(StringUtils::toLower(name));
        }

        return std::string();
    }

    bool    contains(const char * name) const {
        return (count(StringUtils::toLower(name)) > 0 ? true : false);
    }

protected:

    std::string &  operator [] (std::string && name) {
        return std::map<std::string,std::string>::operator [] (name);
    }

    std::string & operator [] (const std::string & name) {        
        return std::map<std::string, std::string>::operator [] (name);
    }
};
