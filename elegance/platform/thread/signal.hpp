#pragma once

#include <set>
#include <platform/thread/mutex.hpp>

namespace face2wind {

enum class SignalType
{
  INTERRUPT,
  COUNT
};
  
class ISignal
{
public:
  ISignal(SignalType type);
  virtual ~ISignal();
  
  bool CheckType(SignalType type);
  void AddType(SignalType type);
  void RemoveType(SignalType type);
  
  virtual void OnReceive(SignalType type) = 0;
  
private:
  std::set<SignalType> type_set_;
  
protected:
  void *param_;
  Mutex mutex_;
};

class SignalManager_
{
public:
  static SignalManager_ & GetInstance();
  ~SignalManager_();
  
  void Register(ISignal *signal);
  void Unregister(ISignal *signal);
  void UpdateSignal();
  
protected:
  SignalManager_();
  
  static void Handle(int sig);
  
  std::set<ISignal*> signal_set_;
  Mutex mutex_;
};

}
