#include <platform/thread/thread.hpp>

namespace face2wind {

#ifdef __LINUX__

Thread::Thread() : thread_id_(0), cur_task_(nullptr), running_(false)
{
}

Thread::~Thread()
{
  this->Detach();  
}

bool Thread::Run(IThreadTask *func, unsigned int stack_size)
{
	if (0 != thread_id_)
		return false;

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, stack_size);

  cur_task_ = func;
  int ret = pthread_create(&thread_id_, &attr, &Thread::StartRoutine, this);
  pthread_attr_destroy(&attr);

  if (0 == ret)
	running_ = true;

  return (0 == ret);
}

bool Thread::Join()
{
  if (0 == thread_id_)
    return false;

  int ret = pthread_join(thread_id_, nullptr);
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
  
  running_ = false;
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
	Thread *thread = (Thread*)param;
	if (nullptr == thread || nullptr == thread->cur_task_)
		return 0;

	thread->cur_task_->Run();

	delete thread->cur_task_;
	thread->cur_task_ = nullptr;
	thread->Detach();
	thread->running_ = false;
	return static_cast<ThreadReturn>(0);
}

#endif

#ifdef __WINDOWS__

Thread::Thread() : thread_id_(0), cur_task_(nullptr), running_(false), thread_handle_(nullptr)
{
}

Thread::~Thread()
{
  this->Detach();
}

bool Thread::Run(IThreadTask *func, unsigned int stack_size)
{
  if (nullptr != thread_handle_)
    return false;

  cur_task_ = func;
  thread_handle_ = CreateThread(nullptr, stack_size, &Thread::StartRoutine, this, 0, &thread_id_);
  if (nullptr == thread_handle_)
    return false;

  running_ = true;
  return true;
}

bool Thread::Join()
{
  if (WaitForSingleObject(thread_handle_, INFINITE) == WAIT_OBJECT_0)
  {
    CloseHandle(thread_handle_);
    thread_handle_ = nullptr;
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
  thread_handle_ = nullptr;
  thread_id_ = 0;

  running_ = false;
  return (0 != ret);
}

bool Thread::Detach()
{
  if (nullptr == thread_handle_)
    return false;

  BOOL ret = CloseHandle(thread_handle_);
  thread_handle_ = nullptr;
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
  Thread *thread = (Thread*)param;
  if (nullptr == thread || nullptr == thread->cur_task_)
    return 0;

  thread->cur_task_->Run();

  delete thread->cur_task_;
  thread->cur_task_ = nullptr;
  thread->Detach();
  thread->running_ = false;
  return static_cast<ThreadReturn>(0);
}

#endif

}
