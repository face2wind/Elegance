#ifndef __MUTEX_HPP__
#define __MUTEX_HPP__

#include <platform/platform_def.hpp>

#ifdef __LINUX__
#include <pthread.h>
#endif

#ifdef __WINDOWS__
#include <windows.h>
#endif

namespace face2wind {

class Mutex
{
 public:
  Mutex();
  ~Mutex();

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

#endif
