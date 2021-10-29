// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#include <atomic>

class SharedData
{
public:
    mutable std::atomic_int ref;

    inline SharedData() noexcept : ref(0) { }
    inline SharedData(const SharedData &) noexcept : ref(0) { }

    SharedData &operator=(const SharedData &) = delete;
    ~SharedData() = default;
};

template <class T> class SharedDataPointer {
public:
    typedef T Type;
    typedef T *pointer;

    inline void detach() { if (d && d->ref != 1) detach_helper(); }
    inline T &operator*() { detach(); return *d; }
    inline const T &operator*() const { return *d; } //-V659
    inline T *operator->() { detach(); return d; }
    inline const T *operator->() const { return d; } //-V659
    inline operator T *() { detach(); return d; }
    inline operator const T *() const { return d; } //-V659
    inline T *data() { detach(); return d; }
    inline const T *data() const { return d; } //-V659
    inline const T *constData() const { return d; }

    inline bool operator==(const SharedDataPointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const SharedDataPointer<T> &other) const { return d != other.d; }

    inline SharedDataPointer() { d = nullptr; }
    inline ~SharedDataPointer() { if (d && !(--d->ref)) delete d; }

    explicit SharedDataPointer(T *data) noexcept;
    inline SharedDataPointer(const SharedDataPointer<T> &o) : d(o.d) { if (d) d->ref++; }
    inline SharedDataPointer<T> & operator=(const SharedDataPointer<T> &o) {
        if (o.d != d) {
            if (o.d)
                o.d->ref++;
            T *old = d;
            d = o.d;
            if (old && !(--old->ref))
                delete old;
        }
        return *this;
    }
    inline SharedDataPointer &operator=(T *o) {
        if (o != d) {
            if (o)
                o->ref++;
            T *old = d;
            d = o;
            if (old && !(--old->ref))
                delete old;
        }
        return *this;
    }
    SharedDataPointer(SharedDataPointer &&o) noexcept : d(o.d) { o.d = nullptr; }
    inline SharedDataPointer &operator=(SharedDataPointer &&other) noexcept
    {
        SharedDataPointer moved(std::move(other));
        swap(moved);
        return *this;
    }

    inline bool operator!() const { return !d; }

    inline void swap(SharedDataPointer &other) noexcept
    {
        std::swap(d, other.d);
    }

protected:
    T *clone();

private:
    void detach_helper();

    T *d;
};

template <class T> inline bool operator==(std::nullptr_t p1, const SharedDataPointer<T> &p2)
{
    return !p2;
}

template <class T> inline bool operator==(const SharedDataPointer<T> &p1, std::nullptr_t p2)
{
    return !p1;
}

template <class T> SharedDataPointer<T>::SharedDataPointer(T *adata) noexcept
    : d(adata)
{
    if (d) d->ref++;
}

template <class T> T *SharedDataPointer<T>::clone()
{
    return new T(*d);
}

template <class T> void SharedDataPointer<T>::detach_helper()
{
    T *x = clone();
    x->ref++;
    if (!(--d->ref))
        delete d;
    d = x;
}

template <class T> void swap(SharedDataPointer<T> &p1, SharedDataPointer<T> &p2)
{
    p1.swap(p2);
}

#define SHARED_DATA_CLASS_DECLARE( class_name, storage_type, storage ) \
protected: \
    SharedDataPointer<storage_type>    storage; \
public: \
  class_name() : d(new storage_type) { \
    static_assert(std::is_base_of<SharedData, storage_type>::value, \
    "class `" #storage_type "` must be derived from SharedData."); \
  } \
  class_name( const class_name &o ) noexcept : storage( o.storage )   {}; \
  class_name( class_name &&o ) noexcept : d( std::move( o.storage ) ) {}; \
  class_name & operator =( const class_name &o ) noexcept { if( &o != this ) { storage = o.storage; } return *this; } \
  class_name & operator =( class_name &&o ) noexcept { storage = std::move( o.storage ); return *this; } \
  ~class_name() = default;

