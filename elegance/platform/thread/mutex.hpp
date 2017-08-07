#pragma once

#include <elegance/platform/platform_def.hpp>

#ifdef __LINUX__
#include <pthread.h>
#endif

#ifdef __WINDOWS__

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601  // for TryEnterCriticalSection compile
#endif

#include <windows.h>
#endif

namespace face2wind {

class ThreadPool;

class Mutex
{
 public:
  Mutex();
  ~Mutex();

  friend class ThreadPool;

  bool Lock();
  bool TryLock();
  bool Unlock();

 private:
#ifdef __LINUX__
  pthread_mutex_t lock_;
#endif

#ifdef __WINDOWS__
  CRITICAL_SECTION critical_section_;
#endif
};

}


