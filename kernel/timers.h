// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <functional>
#include <map>
#include <chrono>
#include <optional>
#include <thread>
#include <mutex>
#include <atomic>

#include "singleton.h"
#include "threadutils.h"

typedef std::function<void()> timerCallback;
typedef std::chrono::time_point<std::chrono::steady_clock>  ExecutionTimestamp;

struct timerInfo {
    int                                   handle  = 0;
    int                                   timeout = 0;
    std::string                           name;
    timerCallback                         callback = nullptr;
    std::optional<ExecutionTimestamp>     executedAt;
    bool                                  isSingleShot = false;
    bool                                  isRunning    = false;
};


class TimerPool {
public:
    TimerPool(const std::string & name = std::string());
    ~TimerPool();

    void    processEvents(int totalTimeoutMs = 50);

    int     create(int timeout, timerCallback f, const std::string & name = std::string());
    int     singleShot(int timeout, timerCallback f, const std::string & name = std::string());
    int     start(int timeout, timerCallback f, const std::string & name = std::string());
    
    void    start(int timerHandle);
    void    stop(int timerHandle);
    void    remove(int timerHandle);

    bool    isActive(int timerHandle) const;
    bool    isSingleShot(int timerHandle) const;

    int     handle(const std::string & name) const;

    static  TimerPool & globalPool() {
        return Singleton<TimerPool>::instance();
    }

    void    activatePool(int aliveTimeout = 500);
    void    deactivatePool();

protected:

    void    threadFunc(int nTimeout);
    int     createTimer(int timeout, timerCallback f, bool isSingleShot, bool isRunning, const std::string & name);

protected:
    std::string               m_name;
    std::map<int, timerInfo>  m_timers;
    std::thread               m_poolThread;
    std::recursive_mutex      m_poolLock;
    std::atomic_bool          m_bExit = false;
    int                       m_nIndex = 1000;

    std::atomic_bool          m_bDirtyFlag = false;
};
