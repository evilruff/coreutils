// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <map>

class MapUtils { 
public:

    template< typename Sk, typename Dk, typename Sv>    static      void
        convertKeys(const std::map<Sk, Sv> & s, std::map<Dk, Sv> & d, std::function<Dk(Sk v)> f) {
        d.clear();
        for (auto it = s.begin(); it != s.end(); ++it) {
            d[f(it->first)] = it->second;
        }
    }

    template< typename Sk, typename Sv>    static      void
        stringifyKeys(const std::map<Sk, Sv> & s, std::map<std::string, Sv> & d) {
        d.clear();
        for (auto it = s.begin(); it != s.end(); ++it) {
            d[std::to_string(it->first)] = it->second;
        }
    }

    template< typename Sk, typename Dk> static std::vector<Dk> values(const std::map<Sk, Dk> & m) {
        std::vector<Dk> result;
        for (const auto & it : m) {
            result.push_back(it.second);
        }
        return result;
    }

    template< typename Sk, typename Dk> static std::vector<Sk> keys(const std::map<Sk, Dk> & m) {
        std::vector<Sk> result;
        for (const auto & it : m) {
            result.push_back(it.first);
        }
        return result;
    }
};