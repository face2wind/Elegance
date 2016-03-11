#ifndef __THREAD_HPP__
#define __THREAD_HPP__

#ifdef WIN32
#include <windows.h>
typedef DWORD ThreadID;
#endif

#ifdef __linux__
#include <pthread.h>
typedef pthread_t ThreadID;
typedef void* ThreadReturn;
#endif

namespace face2wind {

class Thread
{
 public:
  Thread();
  ~Thread();

  bool Run();

  bool Join();
  bool Terminate();

  void Detach();
  ThreadID GetThreadID() const;

 private:

  #ifdef __linux__
  static ThreadReturn StartRoutine(void *param);
  #endif

 private:
  ThreadID m_pid;
};

}

#endif
