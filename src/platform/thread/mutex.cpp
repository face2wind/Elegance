#include <platform/thread/mutex.hpp>

namespace face2wind {

#ifdef __LINUX__

Mutex::Mutex()
{
pthread_mutex_init(&lock_, NULL);
}

Mutex::~Mutex()
{
pthread_mutex_destroy(&lock_);
}

bool Mutex::Lock()
{
return (0 == pthread_mutex_lock(&lock_));
}

bool Mutex::TryLock()
{
return (0 == pthread_mutex_trylock(&lock_));
}
bool Mutex::Unlock()
{
return (0 == pthread_mutex_unlock(&lock_));
}
#endif


#ifdef __WINDOWS__

Mutex::Mutex()
{
InitializeCriticalSection(&critical_section_);
}
Mutex::~Mutex()
{
DeleteCriticalSection(&critical_section_);
}

bool Mutex::Lock()
{
EnterCriticalSection(&critical_section_);
return true;
}
bool Mutex::TryLock()
{
return (TryEnterCriticalSection(&critical_section_));
}
bool Mutex::Unlock()
{
LeaveCriticalSection(&critical_section_);
return true;
}

#endif

}

