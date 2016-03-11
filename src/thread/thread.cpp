#include "thread.hpp"

namespace face2wind {

Thread::Thread()
{
}

Thread::~Thread()
{

}

bool Thread::Run()
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
  return m_pid;
}

ThreadReturn Thread::StartRoutine(void *param)
{
  return NULL;
}

}
