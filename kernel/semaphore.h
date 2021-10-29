// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore (int count_ = 0)
        : count(count_) {}

    inline void notify()
    {
        std::unique_lock<std::mutex> lock(mtx);
        count++;
        cv.notify_one();
    }

    inline void wait()
    {
        std::unique_lock<std::mutex> lock(mtx);

        while(count == 0){
            cv.wait(lock);
        }
        count--;
    }

    inline bool waitFor(int msecTimeout)
    {
        std::unique_lock<std::mutex> lock(mtx);

        while (count == 0) {
            if (cv.wait_for(lock, std::chrono::milliseconds(msecTimeout)) == std::cv_status::timeout) {
                return false;
            }
        }
        count--;

        return true;
    }

private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;
};