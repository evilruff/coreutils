// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "spdlog/pattern_formatter.h"
#include "context.h"

#include <vector>
#include <iomanip>

class LocalDateTimeFormatter : public spdlog::custom_flag_formatter
{
public:
    void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t &dest) override
    {
        DateTime    dt;

        dt = ContextGuard::unlockedContext().clock().currentTime();

        std::string timeStamp = TimeUtils::formatTimestamp(dt);
        dest.append(timeStamp.data(), timeStamp.data() + timeStamp.size());
    }

    std::unique_ptr<custom_flag_formatter> clone() const override
    {
        return spdlog::details::make_unique<LocalDateTimeFormatter>();
    }
};

