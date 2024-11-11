// Description: Cross platform definitions and includes
// makes pthreads usable in Windows and UNIX systems
#ifndef CROSS_PLATFORM_H
#define CROSS_PLATFORM_H

#include "libs.h"

#ifdef _WIN32

#include <windows.h>

//main function is WinMain
#define ENTRY_POINT WinMain

//pthread type
typedef HANDLE pthread_t;
//pthread mutex type
typedef CRITICAL_SECTION pthread_mutex_t;
//pthread mutex init
#define pthread_mutex_init(mutex, ptr) InitializeCriticalSection(mutex)
//pthread mutex lock
#define pthread_mutex_lock EnterCriticalSection
//pthread mutex unlock
#define pthread_mutex_unlock LeaveCriticalSection
//pthread mutex destroy
#define pthread_mutex_destroy DeleteCriticalSection

#else

#include <pthread.h>
//main function is main
#define ENTRY_POINT main

#endif


#endif // CROSS_PLATFORM_H