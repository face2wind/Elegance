#pragma once

#include "network_def.hpp"
#include <platform/platform_def.hpp>

#ifdef __LINUX__
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#endif

namespace face2wind {

class ISocketHandler;

class SocketAccept
{
 public:
  SocketAccept();
  ~SocketAccept();

  bool Init();
  void ResetHandler(ISocketHandler *handler = nullptr);

  bool Listen(Port port);
  bool Write(IPAddr ip, Port port, char *data, int length);

 protected:
  ISocketHandler *handler_;

  bool listening_;
  
#ifdef __LINUX__
  static const int MAX_EPOLL_EVENTS = 1024;
  
  int local_sock_;

  struct epoll_event epoll_event_list_;
  int epoll_fd_;
#endif
  
};

}
