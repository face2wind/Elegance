#include <platform/common/timer.hpp>
#include <platform/platform_def.hpp>

#ifdef __LINUX__
#include <unistd.h>
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
  Sleep(milliseconds);
  #endif
}

}
