#pragma once

#include <elegance/platform/platform_def.hpp>

#ifdef __LINUX__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

#ifdef __WINDOWS__
#include <windows.h>
#endif

namespace face2wind {

class SystemInfo
{
 public:
  static int GetCPUNum();
};

}
