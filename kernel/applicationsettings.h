// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <string>

#include "spdlog/spdlog.h"

#include "constants.h"
#include "classutils.h"

class CommandLineApplicationSettings {
public:
    CommandLineApplicationSettings()  = default;
    ~CommandLineApplicationSettings() = default;


    std::string         configPath          = DEFAULT_CONFIG_PATH;
    std::string         configFileName      = DEFAULT_CONFIG_FILENAME;   
    std::string         ledsFileName        = DEFAULT_LEDS_CONFIG_FILENAME;    
    std::string         kitConfigFileName   = DEFAULT_KIT_CONFIG_FILENAME;

    std::string         scriptIdent;

    bool                daemonize       = false;
    bool                checkConfig     = false;
    bool                emulateSerial   = false;
    int                 debugLogLevel   = spdlog::level::info;
  
    DISABLE_COPY_AND_MOVE(CommandLineApplicationSettings);
};

