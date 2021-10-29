// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "timeutils.h"

#include <ctime>

#include "spdlog/spdlog.h"

#include "stringutils.h"


std::string     TimeUtils::format(const char * fmt, const std::chrono::time_point<std::chrono::system_clock> & t) {    
    std::time_t timeT = std::chrono::system_clock::to_time_t(t);
    std::tm tmT = *std::gmtime(&timeT);

    char output[100];
    if (std::strftime(output, sizeof(output), fmt, &tmT)) {
        return std::string(output);
    }

    return std::string();
}

std::string     TimeUtils::formatTimestamp(time_t t) {
    
    std::tm tmT = *std::gmtime(&t);

    char output[100];
    if (std::strftime(output, sizeof(output), "%Y-%m-%d %H:%M:%S.", &tmT)) {
        std::string result(output);
        result += StringUtils::format("%.03d", t % 1000);
        return result;
    }

    return std::string();
}

std::string     TimeUtils::formatTimestamp(const std::chrono::time_point<std::chrono::system_clock> & t) {
    std::time_t timeT = std::chrono::system_clock::to_time_t(t);
    std::tm tmT = *std::gmtime(&timeT);

    char output[100];
    if (std::strftime(output, sizeof(output), "%Y-%m-%d %H:%M:%S.", &tmT)) {
        std::string result(output);
        result += StringUtils::format("%.03d", std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()) % 1000);
        return result;
    }

    return std::string();
}

std::string     TimeUtils::timepointFromSeconds(int s) {
    return StringUtils::format("%02d:%02d:%02d", s / 3600, (s % 3600) / 60, s % 60);
}

int             TimeUtils::timepointToSeconds(const char * s) {
    int     hh, mm, ss;
    sscanf(s, "%d:%d:%d", &hh, &mm, &ss);
    return (ss + mm * 60 + hh * 60 * 60);
}

LocalClock::LocalClock() {    
    m_markSystemTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_markUserTime   = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

unsigned long long      LocalClock::currentSystemMsSinceEpoch()  {        
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();    
}

DateTime   LocalClock::currentTime() const {
    
    m_timeLock.lock();
    unsigned long long      systemTime = m_markSystemTime;
    unsigned long long      markTime   = m_markUserTime;
    m_timeLock.unlock();

    unsigned long long diffSeconds                       = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - systemTime;

    std::chrono::milliseconds currentTimeDuration(markTime);
    std::chrono::time_point<std::chrono::system_clock> currentTime(currentTimeDuration);

    std::chrono::time_point<std::chrono::system_clock>      currentTimeValue = currentTime + std::chrono::milliseconds(diffSeconds);
    
    return currentTimeValue;
}

time_t     LocalClock::currentPosixTime() const {
    return std::chrono::system_clock::to_time_t(currentTime());
}

void    LocalClock::setUserDateTime(const std::chrono::time_point<std::chrono::system_clock> & t) {  
    
    m_timeLock.lock();
    m_markUserTime   = std::chrono::duration_cast<std::chrono::milliseconds>(t.time_since_epoch()).count();
    m_markSystemTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_timeLock.unlock();

    spdlog::info("LCS time settings adjusted to {} GMT", TimeUtils::format("%c", t));
}

void       LocalClock::setUserDateTime(time_t value) {
    m_timeLock.lock();
    m_markUserTime   = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(value).time_since_epoch()).count();
    m_markSystemTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_timeLock.unlock();

    spdlog::info("LCS time settings adjusted to {} GMT", TimeUtils::format("%c", std::chrono::system_clock::from_time_t(value)));
}

void       LocalClock::setUserDate(int d, int m, int y) {
    time_t timeT   = currentPosixTime();
    tm   *  timeTm = gmtime(&timeT);
    timeTm->tm_mday = d;
    timeTm->tm_mon  = m-1;
    timeTm->tm_year = y;
#ifdef WIN32
    timeT = _mkgmtime(timeTm);
#else
    timeT = timegm(timeTm);
#endif

    m_timeLock.lock();
    m_markUserTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(timeT).time_since_epoch()).count();
    m_markSystemTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_timeLock.unlock();

    spdlog::info("LCS time settings adjusted to {} GMT", TimeUtils::format("%c", std::chrono::system_clock::from_time_t(timeT)));
}

void       LocalClock::setUserTime(int h, int m, int s) {
    time_t timeT = currentPosixTime();
    tm   *  timeTm = gmtime(&timeT);
    timeTm->tm_hour = h;
    timeTm->tm_min  = m;
    timeTm->tm_sec  = s;

#ifdef WIN32
    timeT = _mkgmtime(timeTm);
#else
    timeT = timegm(timeTm);
#endif

    m_timeLock.lock();
    m_markUserTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::from_time_t(timeT).time_since_epoch()).count();
    m_markSystemTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    m_timeLock.unlock();

    spdlog::info("LCS time settings adjusted to {} GMT", TimeUtils::format("%c", std::chrono::system_clock::from_time_t(timeT)));
}

long  long  ElapsedTimer::elapsed() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_startedAt).count();
}

void    ElapsedTimer::reset() {
    m_startedAt = std::chrono::steady_clock::now();
}
