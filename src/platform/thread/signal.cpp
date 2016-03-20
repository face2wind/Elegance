#include <platform/platform_def.hpp>
#include <platform/thread/signal.hpp>

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
    return type_set_.find(type) != type_set_.end();
  }
  
  void ISignal::AddType(SignalType type)
  {
    type_set_.insert(type);
  }
  
  SignalManager_ & SignalManager_::GetInstance()
  {
    static SignalManager_ instance;
    return instance;
  }
  
  SignalManager_::SignalManager_()
  {
    std::cout<<"add signal"<<std::endl;
    signal(SIGINT, SignalManager_::Handle);    
  }
  
  SignalManager_::~SignalManager_()
  {
    std::cout<<"remove signal"<<std::endl;
    signal(SIGINT, SIG_DFL);
  }
  
  void SignalManager_::Register(ISignal *signal)
  {
    if (signal_set_.find(signal) == signal_set_.end())
       signal_set_.insert(signal);
  }
  
  void SignalManager_::Unregister(ISignal *signal)
  {
    if (signal_set_.find(signal) != signal_set_.end())
       signal_set_.erase(signal);
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
