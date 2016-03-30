#pragma once

#include "network_def.hpp"

#ifdef __LINUX__
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <string.h>
#endif

namespace face2wind {

class ISocketHandler;

class SocketConnect
{
 public:
  SocketConnect();
  ~SocketConnect();
  
  void ResetHandler(ISocketHandler *handler = nullptr);

  bool Connect(IPAddr ip, Port port);
  bool Write(char *data, int length);

 protected:
  ISocketHandler *handler_;

  bool running_;
  
#ifdef __LINUX__
  int local_sock_;

  struct epoll_event epoll_event_list_[MAX_EPOLL_EVENTS];
  int epoll_fd_;
#endif
};

}
