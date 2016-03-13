#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#include <platform/platform_def.hpp>

namespace face2wind {

#ifdef __LINUX__

#include <pthread.h>
#include <signal.h>

typedef pthread_t ThreadID;
typedef void* ThreadReturn;

#endif

#ifdef __WINDOWS__

#include <windows.h>
typedef DWORD ThreadID;
typedef DWORD ThreadReturn;

#endif

class Thread
{
 public:
  typedef ThreadReturn (*Func)(void *);

  struct ThreadParam
  {
    Func func;
    void *param;
  };
  
 public:
  Thread();
  ~Thread();

  bool Run(Func func, void *param, unsigned int stack_size = 0);

  bool Join();
  bool Terminate();
  bool Detach();
  
  ThreadID GetThreadID() const;
  ThreadID GetCurrentThreadID() const;

 private:
#ifdef __LINUX__
  static ThreadReturn StartRoutine(void *param);
#endif

#ifdef __WINDOWS__
  static ThreadReturn CALLBACK StartRoutine(void *param);
#endif
  
  
 private:
  ThreadID thread_id_;

#ifdef __WINDOWS__
  HANDLE thread_handle_;
#endif
};

}

#endif
