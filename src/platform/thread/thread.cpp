#include <platform/thread/thread.hpp>

namespace face2wind {

#ifdef __LINUX__

Thread::Thread() : thread_id_(0)
{
}

Thread::~Thread()
{
  this->Detach();  
}

bool Thread::Run(ThreadTask *func, unsigned int stack_size)
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, stack_size);
  
  int ret = pthread_create(&thread_id_, &attr, &Thread::StartRoutine, func);
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

ThreadID Thread::GetCurrentThreadID()
{
  return pthread_self();
}

ThreadReturn Thread::StartRoutine(void *param)
{
  ThreadTask *thread_f = (ThreadTask*)param;
  if (NULL == thread_f)
    return 0;

  thread_f->Run();

  delete thread_f;
  return static_cast<ThreadReturn>(0);
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

bool Thread::Run(ThreadTask *func, unsigned int stack_size)
{
  if (NULL != thread_handle_)
    return false;

  thread_handle_ = CreateThread(NULL, stack_size, &Thread::StartRoutine, func, 0, &thread_id_);
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
  BOOL ret = (0 != TerminateThread(thread_handle_, 0));
  thread_handle_ = NULL;
  thread_id_ = 0;

  return (0 != ret);
}

bool Thread::Detach()
{
  if (NULL == thread_handle_)
    return false;

  BOOL ret = CloseHandle(thread_handle_);
  thread_handle_ = NULL;
  thread_id_ = 0;
  
  return (0 != ret);
}

ThreadID Thread::GetThreadID() const
{
  return thread_id_;
}

ThreadID Thread::GetCurrentThreadID()
{
  return GetCurrentThreadId();
}

ThreadReturn CALLBACK Thread::StartRoutine(void *param)
{
  ThreadTask *thread_f = (ThreadTask*)param;
  if (NULL == thread_f)
    return 0;

  thread_f->Run();

  delete thread_f;
  return static_cast<ThreadReturn>(0);
}

#endif

}
