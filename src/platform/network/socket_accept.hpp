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
#include <queue>
#include <winsock2.h>
#include <Windows.h>
#pragma comment(lib, "Ws2_32.lib") // Socket编程需用的动态链接库
//#pragma comment(lib, "Kernel32.lib") // IOCP需要用到的动态链接库

namespace face2wind {
	
	class SocketAccept;
	enum class IOCPHandleType
	{
		SEND,
		RECV
	};

	typedef struct
	{
		OVERLAPPED overlapped;
		WSABUF databuff;
		CHAR buffer[MAX_SOCKET_MSG_BUFF_LENGTH];
		DWORD bytesTransferred;
		IOCPHandleType type;
		SocketAccept *accept_ptr;
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
  std::map<Endpoint, SOCKET> endpoint_sock_map_;
  std::map<SOCKET, Endpoint> sock_endpoint_map_;
  std::map<SOCKET, std::queue<LPPER_IO_OPERATION_DATA> > send_queue_map_;

  static DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);

  bool all_send_;
#endif
  
};

}
