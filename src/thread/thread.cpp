#include "thread.hpp"

namespace face2wind {

#ifdef __LINUX__

Thread::Thread()
{
}

Thread::~Thread()
{

}

bool Thread::Run(Func func, void *param, unsigned int stack_sizef)
{
  return true;
}

bool Thread::Join()
{
  return true;
}

bool Thread::Terminate()
{
  return true;
}

void Thread::Detach()
{

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
  return NULL;
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

void Thread::Detach()
{
	if (NULL != m_thread_handle)
	{
		CloseHandle(m_thread_handle);
		m_thread_handle = NULL;
		m_thread_id = 0;
	}
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
