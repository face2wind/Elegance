#include <elegance/platform/thread/signal.hpp>

#include <iostream>
#include <signal.h>

namespace face2wind {
  
ISignal::ISignal(SignalType type)
{
  this->AddType(type);
    
  SignalManager_::GetInstance().Register(this);
}
  
ISignal::~ISignal()
{
  SignalManager_::GetInstance().Unregister(this);
}
  
bool ISignal::CheckType(SignalType type)
{
  mutex_.Lock();
  bool has_type = (type_set_.find(type) != type_set_.end());
  mutex_.Unlock();
  
  return has_type;
}
  
void ISignal::AddType(SignalType type)
{
  mutex_.Lock();
  type_set_.insert(type);
  mutex_.Unlock();
  
  SignalManager_::GetInstance().UpdateSignal();
}

void ISignal::RemoveType(SignalType type)
{
  mutex_.Lock();
  type_set_.erase(type);
  mutex_.Unlock();
  
  SignalManager_::GetInstance().UpdateSignal();
}

SignalManager_ & SignalManager_::GetInstance()
{
  static SignalManager_ instance;
  return instance;
}
  
SignalManager_::SignalManager_()
{
  //signal(SIGINT, SignalManager_::Handle);
}
  
SignalManager_::~SignalManager_()
{
  //signal(SIGINT, SIG_DFL);
}
  
void SignalManager_::Register(ISignal *signal)
{
  mutex_.Lock();
  if (signal_set_.find(signal) == signal_set_.end())
    signal_set_.insert(signal);
  mutex_.Unlock();
  
  this->UpdateSignal();
}
  
void SignalManager_::Unregister(ISignal *signal)
{
  mutex_.Lock();
  if (signal_set_.find(signal) != signal_set_.end())
    signal_set_.erase(signal);
  mutex_.Unlock();
  
  this->UpdateSignal();
}

void SignalManager_::UpdateSignal()
{
  mutex_.Lock();
  
  auto &signal_set = SignalManager_::GetInstance().signal_set_;

  const int signal_type_count = (int)SignalType::COUNT;
  bool signal_status_list[signal_type_count]; memset(signal_status_list, 0, sizeof(signal_status_list));
  for (const auto &sig_p : signal_set)
    for (int signal_type = 0; signal_type < signal_type_count; ++ signal_type)
      if (sig_p->CheckType((SignalType)signal_type))
        signal_status_list[signal_type] = true;

  static const int real_signal_value_list[(int)SignalType::COUNT] = {SIGINT};
  for (int signal_type = 0; signal_type < signal_type_count; ++ signal_type)
  {
    signal(real_signal_value_list[signal_type], SIG_DFL);
    if (signal_status_list[signal_type])
      signal(signal_status_list[signal_type], SignalManager_::Handle);
  }

  mutex_.Unlock();
}
  
void SignalManager_::Handle(int sig)
{
  auto &signal_set = SignalManager_::GetInstance().signal_set_;
  for (const auto &sig_p : signal_set)
  {
    if(sig_p->CheckType(SignalType::INTERRUPT))
      sig_p->OnReceive(SignalType::INTERRUPT);
  }
}


}
