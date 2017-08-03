#include <platform/common/timer.hpp>
#include <platform/platform_def.hpp>

#ifdef __LINUX__
#include <unistd.h>
#include <sys/time.h>
#endif

#ifdef __WINDOWS__
#include <windows.h>
#endif

namespace face2wind {

void Timer::Sleep(unsigned int milliseconds)
{
#ifdef __LINUX__
  usleep(milliseconds * 1000);
#endif

#ifdef __WINDOWS__
  ::Sleep(milliseconds);
#endif
}

time_t Timer::GetNowTimeS()
{
  return time(nullptr);
}

unsigned long long Timer::GetNowTimeMS()
{
#ifdef __LINUX__
  struct timeval now_time;
  gettimeofday(&now_time, nullptr);
  return now_time.tv_sec * 1000 + now_time.tv_usec / 1000;
#endif
#ifdef __WINDOWS__
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;
  GetLocalTime(&wtm);
  tm.tm_year     = wtm.wYear - 1900;
  tm.tm_mon     = wtm.wMonth - 1;
  tm.tm_mday     = wtm.wDay;
  tm.tm_hour     = wtm.wHour;
  tm.tm_min     = wtm.wMinute;
  tm.tm_sec     = wtm.wSecond;
  tm. tm_isdst    = -1;
  clock = mktime(&tm);
  return clock * 1000 + wtm.wMilliseconds;
#endif
}

}
