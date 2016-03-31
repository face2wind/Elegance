#pragma once

#include "network_def.hpp"

#ifdef __LINUX__
#include <cstring>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
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
  IPAddr remote_ip_addr_;
  Port remote_port_;

  struct epoll_event epoll_event_list_[MAX_EPOLL_EVENTS];
  int epoll_fd_;

  char buff_[MAX_SOCKET_MSG_BUFF_LENGTH];
#endif
};

}
