#pragma once

#include <set>
#include <list>
#include <elegance/platform/thread/thread.hpp>
#include <elegance/platform/thread/mutex.hpp>
#include <elegance/platform/thread/signal.hpp>

namespace face2wind {

class ThreadPool;

class ThreadPoolSignal : public ISignal
{
 public:
  ThreadPoolSignal(ThreadPool *pool);
  ~ThreadPoolSignal();
  
  virtual void OnReceive(SignalType type);
  
 private:
  ThreadPool *thread_pool_ptr_;
};
  
class ThreadPoolWorkingTask : public IThreadTask
{
 public:
  virtual void Run();
};

class ThreadPool
{
  const static int MAX_THREAD_NUM_LIMIT = 1000;
  
 public:
  ThreadPool();
  ~ThreadPool();
  
  bool Run(int thread_num = 0);
  bool Stop();

  bool AddTask(IThreadTask *task);
  IThreadTask * GetNextTask();

  void WaitALlThread();

 private:
  bool is_running_;

  Mutex mutex_;
  std::set<Thread*> thread_set_;
  std::list<IThreadTask*> task_list_;
  
  ThreadPoolSignal signal_;

#ifdef __LINUX__
  pthread_cond_t condition_;
#endif

#ifdef __WINDOWS__
  enum EventType
  {
    EVENT_TYPE_AUTO_RESET,
    EVENT_TYPE_MANUAL_RESET,

    EVENT_TYPE_MAX
  };

  HANDLE handle_list_[EVENT_TYPE_MAX];
#endif
};

}

