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

  virtual void OnAccept(IPAddr ip, Port port) = 0;
  virtual void OnConnect(IPAddr ip, Port port) = 0;
  
  virtual void OnRecv(IPAddr ip, Port port, char *data, int length) = 0;
  
  virtual void OnDisconnect(IPAddr ip, Port port) = 0;
};

}
