#pragma once

#include "network_def.hpp"

namespace face2wind {

typedef unsigned short Port;
typedef std::string IPAddr;

class ISocketHandler
{
 public:
  ISocketHandler() {}
  virtual ~ISocketHandler() {}

  void OnAccept(IPAddr ip, Port port) = 0;
  void OnConnect(IPAddr ip, Port port) = 0;
  
  void OnRecv(IPAddr ip, Port port, char *data, int length) = 0;
  
  void OnDisconnect(IPAddr ip, Port port) = 0;
};

}
