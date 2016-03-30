#pragma once

#include <string>
#include <platform/platform_def.hpp>

namespace face2wind {

typedef unsigned short Port;
typedef std::string IPAddr;

#ifdef __LINUX__
  static const int MAX_EPOLL_EVENTS = 1024;
  static const int MAX_BACKLOG = 128;
#endif

struct Endpoint
{
  Endpoint(IPAddr ip, Port port) : ip_addr(""), port(0) {}
  
  IPAddr ip_addr;
  Port port;
};

}
