#pragma once

#include <set>
#include <list>
#include <platform/thread/thread.hpp>
#include <platform/thread/mutex.hpp>

namespace face2wind {

class ThreadPoolWorkingTask : public ThreadTask
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

  bool AddTask(ThreadTask *task);
  ThreadTask * GetNextTask();

 private:
  bool is_running_;

  Mutex mutex_;
  std::set<Thread*> thread_set_;
  std::list<ThreadTask*> task_list_;

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

