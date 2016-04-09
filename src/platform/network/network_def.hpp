#pragma once

#include <platform/platform_def.hpp>
#include <iostream>
#include <string>

namespace face2wind {

typedef unsigned short Port;
typedef std::string IPAddr;

static const int MAX_SOCKET_MSG_BUFF_LENGTH = 5;
static const int MAX_BACKLOG = 128;

#ifdef __LINUX__
static const int MAX_EPOLL_EVENTS = 1024;
#endif
#ifdef __WINDOWS__

#endif

struct Endpoint
{
  Endpoint() : ip_addr(""), port(0) {}
  Endpoint(IPAddr ip, Port port) : ip_addr(ip), port(port) {}
  bool operator <(const Endpoint &other) const
  {
    if (ip_addr.size() < other.ip_addr.size())
      return true;
    else if (ip_addr.size() > other.ip_addr.size())
      return false;
    else
      return port < other.port;
  }
  
  IPAddr ip_addr;
  Port port;
};

}
