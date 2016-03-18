#pragma once

#include <platform/platform_def.hpp>

#ifdef __LINUX__
#include <pthread.h>
#endif

#ifdef __WINDOWS__
#include <windows.h>
#endif

namespace face2wind {

class Condition
{
 public:
  Condition();
  ~Condition();

  void Wait();
  void Wakeup();
  void WakeupAll();

#ifdef __LINUX__
  pthread_cond_t condition_;
#endif

#ifdef __WINDOWS__

#endif
  
};

}
