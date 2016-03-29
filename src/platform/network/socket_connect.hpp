#pragma once

#include "network_def.hpp"

namespace face2wind {

class SocketConnect
{
 public:
  SocketConnect() {}
  ~SocketConnect() {}
  
  bool Connect(IPAddr ip, Port port, int timeout_s = 0);
  bool Write(char *data, int length);

  bool ResetHandler(ISocketHandler *handler = nullptr);

 protected:
  ISocketHandler *handler_;

};

}
