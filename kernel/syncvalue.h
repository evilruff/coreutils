// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <mutex>


#include "classutils.h"

template <typename T>   class SyncValue {
public:
    SyncValue(const T & v = T()) : value(v) {};
   
    SyncValue<T> & operator =(const T & v) { const std::lock_guard<std::recursive_mutex>guard(lock); value = v; return *this; };
    explicit operator T() const { const std::lock_guard<std::recursive_mutex>guard(lock); return value; }
    bool operator !=(const T & other) const { const std::lock_guard<std::recursive_mutex>guard(lock); return other != value; };

    DISABLE_COPY_AND_MOVE(SyncValue);
protected:
    mutable std::recursive_mutex    lock;
    T                               value;
};


