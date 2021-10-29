// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <atomic>
#include <mutex>

typedef std::chrono::time_point<std::chrono::system_clock>      DateTime;
typedef std::chrono::time_point<std::chrono::steady_clock>      MonoliticTime;

class TimeUtils {
public:
    static std::string     format(const char * fmt, const std::chrono::time_point<std::chrono::system_clock> & t);
    static std::string     formatTimestamp(const std::chrono::time_point<std::chrono::system_clock> & t);
    static std::string     formatTimestamp(time_t t);

    static std::string     timepointFromSeconds(int s);
    static int             timepointToSeconds(const char * s);
};

class LocalClock {
public:
    LocalClock();
    ~LocalClock() = default;

    DateTime   currentTime() const;
    time_t     currentPosixTime() const;
    void       setUserDateTime(const std::chrono::time_point<std::chrono::system_clock> & t);
    void       setUserDateTime(time_t value);
    void       setUserDate(int d, int m, int y);
    void       setUserTime(int h, int m, int s);
    bool       isAdjusted() const { return (m_markSystemTime > 0); };

    static unsigned long long      currentSystemMsSinceEpoch();

    LocalClock(const LocalClock &  other) = delete;
    LocalClock(const LocalClock && other) = delete;
    LocalClock & operator = (const LocalClock &  other)  = delete;
    LocalClock & operator = (const LocalClock &&  other) = delete;

protected:
    
    mutable std::recursive_mutex    m_timeLock;
    std::atomic_ullong              m_markSystemTime;
    std::atomic_ullong              m_markUserTime;
};

class ElapsedTimer {
public:
    ElapsedTimer()  { reset(); }
    ElapsedTimer(const ElapsedTimer & other) : m_startedAt(other.m_startedAt) {};
    ElapsedTimer & operator = (const ElapsedTimer & other) { m_startedAt = other.m_startedAt;  return *this; };
    ~ElapsedTimer() = default;
    
    long long    elapsed() const;
    void         reset();
    bool         isExpired(long long timeout) { return elapsed() > timeout; };

protected:
    MonoliticTime       m_startedAt;
};