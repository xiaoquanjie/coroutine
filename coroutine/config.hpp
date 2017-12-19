#ifndef M_COROUTINE_CONFIG_INCLUDE
#define M_COROUTINE_CONFIG_INCLUDE

#define M_COROUTINE_NAMESPACE_BEGIN namespace coroutine{
#define M_COROUTINE_NAMESPACE_END }

#ifndef M_PLATFORM_WIN32
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define M_PLATFORM_WIN32 1
#endif
#endif

#ifndef M_PLATFORM_WIN
#if defined(M_PLATFORM_WIN32) || defined(WIN64)
#define M_PLATFORM_WIN 1
#endif
#endif

#ifdef M_PLATFORM_WIN
#include <windows.h>
#else
#include <pthread.h>
#include <ucontext.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <map>

#endif