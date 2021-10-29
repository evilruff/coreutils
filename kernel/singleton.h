// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once
 
template <typename T> class Singleton
{
public:
    static T & instance()
    {
        static T myInstance;

        return myInstance;
    }

    Singleton<T>(Singleton<T> const&) = delete;             // Copy construct
    Singleton<T>(Singleton<T>&&) = delete;                  // Move construct
    Singleton<T> & operator=(Singleton<T> const&) = delete;  // Copy assign
    Singleton<T> & operator=(Singleton<T> &&) = delete;      // Move assign

    protected:

    Singleton<T>()  = default;
    ~Singleton<T>() = default;
};
