#include <platform/thread/thread.hpp>

namespace face2wind {

#ifdef __LINUX__

Thread::Thread()
{
}

Thread::~Thread()
{

}

bool Thread::Run(Func func, void *param, unsigned int stack_size)
{
  ThreadParam *thread_p = new ThreadParam();
  thread_p->func = func;
  thread_p->param = param;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, stack_size);
  
  int ret = pthread_create(&thread_id_, &attr, &Thread::StartRoutine, thread_p);
  pthread_attr_destroy(&attr);

  return (0 == ret);
}

bool Thread::Join()
{
  if (0 == thread_id_)
    return false;

  int ret = pthread_join(thread_id_, NULL);
  if (0 == ret)
    thread_id_ = 0;

  return 0 == ret;
}

bool Thread::Terminate()
{
  if (0 == thread_id_)
    return false;

  int ret = pthread_kill(thread_id_, SIGKILL);
  if (0 == ret)
    thread_id_ = 0;
  
  return 0 == ret;
}

bool Thread::Detach()
{
  if (0 == thread_id_)
    return false;

  int ret = pthread_detach(thread_id_);
  thread_id_ = 0;

  return 0 == ret;
}

ThreadID Thread::GetThreadID() const
{
  return thread_id_;
}

ThreadID Thread::GetCurrentThreadID() const
{
  return pthread_self();
}

ThreadReturn Thread::StartRoutine(void *param)
{
  ThreadParam *thread_p = (ThreadParam*)param;
  if (NULL == thread_p)
    return 0;

  ThreadReturn ret = thread_p->func(thread_p->param);

  delete thread_p;
  return ret;
}

#endif

#ifdef __WINDOWS__

Thread::Thread() : thread_id_(0), thread_handle_(NULL)
{
}

Thread::~Thread()
{
  this->Detach();
}

bool Thread::Run(Func func, void *param, unsigned int stack_size)
{
  if (NULL != thread_handle_)
    return false;

  ThreadParam *thread_p = new ThreadParam();
  thread_p->func = func;
  thread_p->param = param;

  thread_handle_ = CreateThread(NULL, stack_size, &Thread::StartRoutine, thread_p, 0, &thread_id_);
  if (NULL == thread_handle_)
    return false;

  return true;
}

bool Thread::Join()
{
  if (WaitForSingleObject(thread_handle_, INFINITE) == WAIT_OBJECT_0)
  {
    CloseHandle(thread_handle_);
    thread_handle_ = NULL;
    thread_id_ = 0;
    return true;
  }
  else
  {
    return false;
  }
}

bool Thread::Terminate()
{
  bool ret = (0 != TerminateThread(thread_handle_, 0));
  thread_handle_ = NULL;
  thread_id_ = 0;
  return ret;
}

bool Thread::Detach()
{
  if (NULL == thread_handle_)
    return false;

  bool ret = CloseHandle(thread_handle_);
  thread_handle_ = NULL;
  thread_id_ = 0;
  
  return ret;
}

ThreadID Thread::GetThreadID() const
{
  return thread_id_;
}

ThreadID Thread::GetCurrentThreadID() const
{
  return GetCurrentThreadId();
}

ThreadReturn CALLBACK Thread::StartRoutine(void *param)
{
  ThreadParam *thread_p = (ThreadParam*)param;
  if (NULL == thread_p)
    return 0;

  ThreadReturn ret = thread_p->func(thread_p->param);

  delete thread_p;
  return ret;
}

#endif

}
