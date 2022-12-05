// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "timers.h"

#include "spdlog/spdlog.h"

TimerPool::TimerPool(const std::string & name):
    m_name(name) {
    spdlog::info("TimePool {} initialized", name.size() ? name : "default");
}

TimerPool::~TimerPool() {
    deactivatePool();
    spdlog::info("TimePool destroyed");
}

void    TimerPool::processEvents(int totalTimeoutMs) {
    m_poolLock.lock();

    std::map<int, timerInfo>::iterator it = m_timers.begin();

    ExecutionTimestamp  currentTime = std::chrono::steady_clock::now();

    while (it != m_timers.end()) {        
        timerInfo &    t = it->second;
        std::map<int, timerInfo>::iterator currentTimerIt = it;
        
        ++it;

        if (!t.isRunning)
            continue;

        if (!t.executedAt.has_value()) {
            t.executedAt = currentTime;
            continue;
        }

        
        std::chrono::milliseconds currentTimeout = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - *t.executedAt);
        if (currentTimeout.count() > t.timeout) {
            t.executedAt = currentTime;

            timerCallback callback = t.callback;

            if (t.isSingleShot) {
                it = m_timers.erase(currentTimerIt);
            }

            m_bDirtyFlag = false;
            m_poolLock.unlock();

            callback();

            m_poolLock.lock();
            if (m_bDirtyFlag) {
                m_poolLock.unlock();
                return;
            }            
        }
    }

    m_poolLock.unlock();

    if (totalTimeoutMs) {
        ExecutionTimestamp processTime = std::chrono::steady_clock::now();
        std::chrono::milliseconds currentTimeout = std::chrono::duration_cast<std::chrono::milliseconds>(processTime  - currentTime);
        if (totalTimeoutMs > currentTimeout.count()) {
            m_poolLock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(totalTimeoutMs - currentTimeout.count()));
        }
    }
}

int     TimerPool::create(int timeout, timerCallback f, const std::string & name) {

    spdlog::debug("Timer {} ms created", timeout);

    return createTimer(timeout, f, false, false, name);
}

int     TimerPool::singleShot(int timeout, timerCallback f, const std::string & name ) {
    spdlog::debug("Singleshot timer {} ms created", timeout);

    int nId = createTimer(timeout, f, true, false, name);
    start(nId);

    return nId;
}

int     TimerPool::start(int timeout, timerCallback f, const std::string & name) {
    spdlog::debug("Timer {} ms created", timeout);

    return createTimer(timeout, f, false, true, name);
}

int     TimerPool::createTimer(int timeout, timerCallback f, bool isSingleShot, bool isRunning, const std::string & name) {
    MutexLocker<std::recursive_mutex> lock(m_poolLock);

    timerInfo info;
    info.timeout      = timeout;
    info.handle       = m_nIndex++;
    info.name         = name;
    info.callback     = f;
    info.isSingleShot = isSingleShot;
    info.isRunning    = isRunning; 

    m_timers[info.handle] = info;

    m_bDirtyFlag      = true;

    return info.handle;
}

void    TimerPool::start(int timerHandle) {
    MutexLocker<std::recursive_mutex> lock(m_poolLock);

    if (!m_timers.count(timerHandle))
        return;

    m_timers[timerHandle].isRunning = true;
}

void    TimerPool::stop(int timerHandle) {
    MutexLocker<std::recursive_mutex> lock(m_poolLock);

    if (!m_timers.count(timerHandle))
        return;

    m_timers[timerHandle].isRunning = false;
}

void    TimerPool::remove(int timerHandle) {
    MutexLocker<std::recursive_mutex> lock(m_poolLock);

    m_timers.erase(timerHandle);
}

bool    TimerPool::isActive(int timerHandle) const {
    MutexLocker<std::recursive_mutex> lock(const_cast<TimerPool*>(this)->m_poolLock);

    if (!m_timers.count(timerHandle))
        return false;

    return m_timers.at(timerHandle).isRunning;
}

bool    TimerPool::isSingleShot(int timerHandle) const {
    MutexLocker<std::recursive_mutex> lock(const_cast<TimerPool*>(this)->m_poolLock);

    if (!m_timers.count(timerHandle))
        return false;

    return m_timers.at(timerHandle).isSingleShot;
}

int     TimerPool::handle(const std::string & name) const {
    MutexLocker<std::recursive_mutex> lock(const_cast<TimerPool*>(this)->m_poolLock);

    std::map<int, timerInfo>::const_iterator it = find_if(m_timers.begin(), m_timers.end(), [&name](const std::pair<int, timerInfo>& v) { return v.second.name == name; });
    if (it != m_timers.end()) {
        return (it->second.handle);
    }

    return 0;
}

void    TimerPool::activatePool(int aliveTimeout) {
    m_poolThread = std::thread(&TimerPool::threadFunc, this, aliveTimeout);
#ifdef WIN32
    SetThreadDescription(m_poolThread.native_handle(), L"Timers Thread");
#endif
}

void     TimerPool::threadFunc(int nTimeout) {
    while (!m_bExit) {
        processEvents(nTimeout);
    }
}

void    TimerPool::deactivatePool() {
    m_bExit = true;
    if (m_poolThread.joinable()) {
        m_poolThread.join();
    }
}