#pragma once

#include <string>

namespace face2wind {

typedef unsigned short Port;
typedef std::string IPAddr;

struct Endpoint
{
  Endpoint(IPAddr ip, Port port) : ip_addr(""), port(0) {}
  
  IPAddr ip_addr;
  Port port;
};

}
