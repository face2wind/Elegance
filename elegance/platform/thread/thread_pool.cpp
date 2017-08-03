#include <iostream>

#include <common/debug_message.hpp>
#include <platform/common/system_info.hpp>
#include <platform/thread/thread_pool.hpp>

namespace face2wind {
  
ThreadPoolSignal::ThreadPoolSignal(ThreadPool *pool) : ISignal(SignalType::INTERRUPT)
{
  thread_pool_ptr_ = pool;
}
  
ThreadPoolSignal::~ThreadPoolSignal()
{
}
  
void ThreadPoolSignal::OnReceive(SignalType type)
{
  fDebugWithHead(DebugMessageType::THREAD) << "ThreadPoolSignal::OnReceive("<<int(type)<<")" << fDebugEndl;
    
  if (nullptr != thread_pool_ptr_)
    thread_pool_ptr_->Stop();
}

void ThreadPoolWorkingTask::Run()
{
  ThreadPool *pool = (ThreadPool*)param_;
  if (nullptr == pool)
    return;

  while(true)
  {
    IThreadTask *task = pool->GetNextTask();
    if (nullptr == task)
      break;

    task->Run();
    delete task;
  }
}

ThreadPool::ThreadPool() : is_running_(false), signal_(this)
{
#ifdef __LINUX__
  pthread_cond_init(&condition_, nullptr);
#endif

#ifdef __WINDOWS__
  handle_list_[EVENT_TYPE_AUTO_RESET] = CreateEvent(nullptr, FALSE, FALSE, TEXT("auto_reset_event"));
  handle_list_[EVENT_TYPE_MANUAL_RESET] = CreateEvent(nullptr, TRUE, FALSE, TEXT("manual_reset_event"));
#endif
}

ThreadPool::~ThreadPool()
{
  this->Stop();
}

bool ThreadPool::Run(int thread_num)
{
  if (is_running_)
    return true;
  
  if (thread_num <= 0)
    thread_num = SystemInfo::GetCPUNum();

  if (thread_num <= 0 || thread_num > MAX_THREAD_NUM_LIMIT)
    return false;

  is_running_ = true;
  
  fDebugWithHead(DebugMessageType::THREAD) << "ThreadPool::Run(" << thread_num << ")" << fDebugEndl;
  for (int index = 0; index < thread_num; ++ index)
  {
    ThreadPoolWorkingTask *work_task = new ThreadPoolWorkingTask();
    work_task->SetParam(this);
    
    Thread *tmp_thread = new Thread();
    tmp_thread->Run(work_task);
    thread_set_.insert(tmp_thread);
  }
  
  return true;
}

bool ThreadPool::Stop()
{
  if (!is_running_)
    return true;
  
  mutex_.Lock();
  for (std::list<IThreadTask*>::iterator it = task_list_.begin(); it != task_list_.end(); ++ it)
    delete *it;
  task_list_.clear();
  mutex_.Unlock();
  
  is_running_ = false;
#ifdef __LINUX__
  pthread_cond_broadcast(&condition_);
#endif
#ifdef __WINDOWS__
  SetEvent(handle_list_[EVENT_TYPE_MANUAL_RESET]);
#endif
  
  for (std::set<Thread*>::iterator it = thread_set_.begin(); it != thread_set_.end(); ++ it)
  {
    (*it)->Join();
    delete *it;
  }
  thread_set_.clear();

#ifdef __LINUX__
  pthread_cond_destroy(&condition_);
#endif

#ifdef __WINDOWS__
  CloseHandle(handle_list_[EVENT_TYPE_AUTO_RESET]);
  CloseHandle(handle_list_[EVENT_TYPE_MANUAL_RESET]);
#endif
  signal_.RemoveType(SignalType::INTERRUPT);
  return true;
}

bool ThreadPool::AddTask(IThreadTask *task)
{
  if (nullptr == task)
    return false;
  
  if (!is_running_)
    return false;
  
  mutex_.Lock();

  task_list_.push_back(task);
  
#ifdef __LINUX__
  pthread_cond_signal(&condition_);
#endif
  
#ifdef __WINDOWS__
  SetEvent(handle_list_[EVENT_TYPE_AUTO_RESET]);
#endif
  
  mutex_.Unlock();

  return true;
}

IThreadTask * ThreadPool::GetNextTask()
{
  if (!is_running_)
    return nullptr;
  
  mutex_.Lock();
  
#ifdef __LINUX__
  while (task_list_.empty() && is_running_)
  {
    pthread_cond_wait(&condition_, &mutex_.lock_);
  }
#endif

#ifdef __WINDOWS__
  mutex_.Unlock();
  while (task_list_.empty() && is_running_)
  {
    DWORD wait_result = WaitForMultipleObjects(EVENT_TYPE_MAX, handle_list_, FALSE, INFINITE);
    switch (wait_result)
    {
      case WAIT_OBJECT_0 + EVENT_TYPE_AUTO_RESET:
        //std::cout<<"handle auto"<<std::endl;
        break;
      case WAIT_OBJECT_0 + EVENT_TYPE_MANUAL_RESET:
        //std::cout<<"handle manual"<<std::endl;
        ResetEvent(handle_list_[EVENT_TYPE_MANUAL_RESET]);
        break;
    default: break;
    }
  }
  mutex_.Lock();
#endif

  fDebugWithHead(DebugMessageType::THREAD) << "thread "<<Thread::GetCurrentThreadID()<<" : " << fDebugEndl;
  if (task_list_.empty() || !is_running_)
  {
    fDebugWithHead(DebugMessageType::THREAD) << "father thread stoped, "<<task_list_.size()<<" - "<<is_running_<<fDebugEndl;
    mutex_.Unlock();
    return nullptr;
  }

  fDebugWithHead(DebugMessageType::THREAD) << "GetNextTask succ!!"<< fDebugEndl;
  
  IThreadTask *task = task_list_.front();
  
  task_list_.pop_front();
  
  mutex_.Unlock();
  return task;
}

void ThreadPool::WaitALlThread()
{
	for (auto th : thread_set_)
	{
		th->Join();
	}
}

}
