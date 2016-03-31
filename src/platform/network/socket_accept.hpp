#pragma once

#include "network_def.hpp"
#include <map>

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
  int local_sock_;

  struct epoll_event epoll_event_list_[MAX_EPOLL_EVENTS];
  int epoll_fd_;

  char buff_[MAX_SOCKET_MSG_BUFF_LENGTH];

  std::map<Endpoint, int> endpoint_sock_map_;
  std::map<Endpoint, int> sock_endpoint_map_;
#endif
  
};

}
