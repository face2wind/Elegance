#pragma once

#include "socket_def.hpp"

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
#ifdef __WINDOWS__
#include <queue>

#endif

namespace face2wind {

class ISocketHandler;

class SocketAccept
{
 public:
  SocketAccept();
  ~SocketAccept();

  void ResetHandler(ISocketHandler *handler = nullptr);

  bool Listen(Port port);
  bool Write(IPAddr ip, Port port, const char *data, int length);
  bool Disconnect(IPAddr ip, Port port);

  bool CheckOnHandle(IPAddr ip, Port port);

 protected:
  ISocketHandler *handler_;

  bool listening_;
  Port local_port;

#ifdef __LINUX__
  int local_sock_;

  struct epoll_event epoll_event_list_[MAX_EPOLL_EVENTS];
  int epoll_fd_;

  char buff_[MAX_SOCKET_MSG_BUFF_LENGTH];

  std::map<Endpoint, int> endpoint_sock_map_;
  std::map<int, Endpoint> sock_endpoint_map_;
#endif
#ifdef __WINDOWS__
  std::map<Endpoint, SOCKET> endpoint_sock_map_;
  std::map<SOCKET, Endpoint> sock_endpoint_map_;
  std::map<SOCKET, std::queue<LPPER_IO_OPERATION_DATA> > send_queue_map_;

  static DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);

#endif
  
};

}
