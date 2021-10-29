// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#pragma once

#define DISABLE_COPY(T)		T(const T & other)  = delete; \
				T & operator=(const T &) = delete; 				

#define DISABLE_MOVE(T)		T(T && other) = delete; \
				T & operator=(T &&) = delete;

#define DISABLE_COPY_AND_MOVE(T)	T(const T & other)  = delete; \
					T(T && other) = delete; \
					T & operator=(const T &) = delete; \
					T & operator=(T &&) = delete;

