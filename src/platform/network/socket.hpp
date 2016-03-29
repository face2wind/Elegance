#pragma once

#include "i_socket_handler.hpp"

namespace face2wind {

class Socket : public ISocketHandler
{
 public:
  Socket();
  ~Socket();

  bool Close();

  bool ResetHandler(ISocketHandler *handler = nullptr);

 protected:
  ISocketHandler *handler_;
};

}
