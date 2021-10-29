// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "stringutils.h"
#include <algorithm>

namespace StringUtils {

    bool endsWith(std::string const &str, std::string const &ending) {
        if (str.length() >= ending.length()) {
            return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
        }
        else {
            return false;
        }
    }

    std::string     toUpper(const std::string & orig) {
        std::string s = orig;
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::toupper(c); });
        return s;
    }

    std::string     toLower(const std::string & orig) {
        std::string s = orig;
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    void     toUpper(std::string & s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::toupper(c); });
    }

    void     toLower(std::string & s) {
        std::transform(s.begin(), s.end(), s.begin(),
            [](unsigned char c) { return std::tolower(c); });
    }

    std::vector<std::string> split(const std::string & str, char delimeter, bool skipEmpty, std::function <bool(const std::string & item)> filter) {
        std::vector<std::string>  items;
        std::stringstream ss(str);
        std::string item;

        while (getline(ss, item, delimeter)) {
            if (skipEmpty && item.size() == 0)
                continue;

            if (filter && !filter(item))
                continue;

            items.emplace_back(item);
        }

        return items;
    }

    std::pair<std::string, std::string> splitPair(const std::string & str, char delimeter) {
        std::pair<std::string, std::string>  items;
        std::stringstream ss(str);
        std::string item;

        getline(ss, item, delimeter);
        if (!item.size())
            return items;

        items.first = item;

        getline(ss, item, delimeter);

        items.second = item;

        return items;
    }

}

