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
#ifdef __WINDOWS__
#include <winsock2.h>
#include <Windows.h>

namespace face2wind {
	typedef struct
	{
		OVERLAPPED overlapped;
		WSABUF databuff;
		CHAR buffer[MAX_SOCKET_MSG_BUFF_LENGTH];
		DWORD bytesSend;
		DWORD bytesRecv;
	}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA;

	typedef struct
	{
		SOCKET socket;
	}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;
}
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

 protected:
  ISocketHandler *handler_;

  bool listening_;

#ifdef __LINUX__
  int local_sock_;

  struct epoll_event epoll_event_list_[MAX_EPOLL_EVENTS];
  int epoll_fd_;

  char buff_[MAX_SOCKET_MSG_BUFF_LENGTH];

  std::map<Endpoint, int> endpoint_sock_map_;
  std::map<int, Endpoint> sock_endpoint_map_;
#endif
#ifdef __WINDOWS__
#endif
  
};

}
