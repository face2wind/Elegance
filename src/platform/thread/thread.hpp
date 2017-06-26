#pragma once

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

class IThreadTask
{
 public:
  IThreadTask(): param_(nullptr) {}
  virtual ~IThreadTask() {}

  void SetParam(void *param) { param_ = param; }
  
  virtual void Run() = 0;

 protected:
  void *param_;
};

class Thread
{
 public:
  Thread();
  ~Thread();

  bool Run(IThreadTask *func, unsigned int stack_size = 0);

  bool Join();
  bool Terminate();
  bool Detach();
  
  ThreadID GetThreadID() const;
  static ThreadID GetCurrentThreadID();

  bool IsRunning() { return running_; }

 private:
#ifdef __LINUX__
  static ThreadReturn StartRoutine(void *param);
#endif

#ifdef __WINDOWS__
  static ThreadReturn CALLBACK StartRoutine(void *param);
#endif
  
  
 private:
  ThreadID thread_id_;
  IThreadTask *cur_task_;
  bool running_;

#ifdef __WINDOWS__
  HANDLE thread_handle_;
#endif
};

}

