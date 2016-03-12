#include "thread.hpp"

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
  
  int ret = pthread_create(&m_thread_id, &attr, &Thread::StartRoutine, thread_p);
  pthread_attr_destroy(&attr);

  return (0 == ret);
}

bool Thread::Join()
{
  if (0 == m_thread_id)
    return false;

  int ret = pthread_join(m_thread_id, NULL);
  if (0 == ret)
    m_thread_id = 0;

  return 0 == ret;
}

bool Thread::Terminate()
{
  if (0 == m_thread_id)
    return false;

  int ret = pthread_kill(m_thread_id, SIGKILL);
  if (0 == ret)
    m_thread_id = 0;
  
  return 0 == ret;
}

bool Thread::Detach()
{
  if (0 == m_thread_id)
    return false;

  int ret = pthread_detach(m_thread_id);
  m_thread_id = 0;

  return 0 == ret;
}

ThreadID Thread::GetThreadID() const
{
  return m_thread_id;
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

Thread::Thread() : m_thread_id(0), m_thread_handle(NULL)
{
}

Thread::~Thread()
{
  this->Detach();
}

bool Thread::Run(Func func, void *param, unsigned int stack_size)
{
  if (NULL != m_thread_handle)
    return false;

  ThreadParam *thread_p = new ThreadParam();
  thread_p->func = func;
  thread_p->param = param;

  m_thread_handle = CreateThread(NULL, stack_size, &Thread::StartRoutine, thread_p, 0, &m_thread_id);
  if (NULL == m_thread_handle)
    return false;

  return true;
}

bool Thread::Join()
{
  if (WaitForSingleObject(m_thread_handle, INFINITE) == WAIT_OBJECT_0)
  {
    CloseHandle(m_thread_handle);
    m_thread_handle = NULL;
    m_thread_id = 0;
    return true;
  }
  else
  {
    return false;
  }
}

bool Thread::Terminate()
{
  bool ret = (0 != TerminateThread(m_thread_handle, 0));
  m_thread_handle = NULL;
  m_thread_id = 0;
  return ret;
}

bool Thread::Detach()
{
  if (NULL == m_thread_handle)
    return false;

  bool ret = CloseHandle(m_thread_handle);
  m_thread_handle = NULL;
  m_thread_id = 0;
  
  return ret;
}

ThreadID Thread::GetThreadID() const
{
  return m_thread_id;
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
