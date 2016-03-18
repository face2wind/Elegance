#include <platform/thread/condition.hpp>

namespace face2wind {

#ifdef __LINUX__

Condition::Condition()
{
  pthread_cond_init(&condition_, NULL);
}

Condition::~Condition()
{
  pthread_cond_destroy(&condition_);
}

void Condition::Wait()
{
  
}

void Condition::Wakeup()
{
}

void Condition::WakeupAll()
{
}

#endif

}
