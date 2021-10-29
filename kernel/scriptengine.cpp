// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "scriptengine.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "stringutils.h"

#ifdef WIN32

#define POPEN  _popen
#define PCLOSE _pclose

#else

#define POPEN  popen
#define PCLOSE pclose

#endif

bool             ScriptCore::execute() {
    d->result.clear();

    if (!isValid())
        return false;

    std::vector<char> buffer(d->bufferSize, '\n');
    
    std::string result;
    std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(d->command.c_str(), "r"), PCLOSE);
    if (!pipe) {
        return false;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        d->result += buffer.data();
    }
    return true;
}

