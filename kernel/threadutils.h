// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include "classutils.h"

template <typename T> class   MutexLocker
{
public:
    MutexLocker(T & m):m_mutex(m) {
        m_mutex.lock();
    }

    ~MutexLocker() {
        m_mutex.unlock();
    }

protected:
    T  & m_mutex;

    DISABLE_COPY_AND_MOVE(MutexLocker);
};