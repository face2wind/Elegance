#include <platform/common/system_info.hpp>

namespace face2wind {

int SystemInfo::GetCPUNum()
{
#ifdef __LINUX__
  return sysconf(_SC_NPROCESSORS_ONLN);
  //return get_nprocs();   //GNU fuction 
#endif
  
#ifdef __WINDOWS__
  SYSTEM_INFO info;
  GetSystemInfo(&info);
  return info.dwNumberOfProcessors;  
#endif
}

}
