#pragma once

#include <platform/platform_def.hpp>
#include <iostream>
#include <string>

namespace face2wind {

typedef unsigned short Port;
typedef std::string IPAddr;
typedef int NetworkID;

static const int MAX_SOCKET_MSG_BUFF_LENGTH = 5;// 1024 * 4;
static const int MAX_BACKLOG = 128;

#ifdef __LINUX__
static const int MAX_EPOLL_EVENTS = 1024;
#endif

#ifdef __WINDOWS__

#include <winsock2.h>
#include <Windows.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib") // Socket编程需用的动态链接库
//#pragma comment(lib, "Kernel32.lib") // IOCP需要用到的动态链接库

class SocketAccept;
class SocketConnect;

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
  SocketConnect *connect_ptr;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA;

typedef struct
{
  SOCKET socket;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

#endif

struct Endpoint
{
  Endpoint() : ip_addr(""), port(0) {}
  Endpoint(IPAddr ip, Port port) : ip_addr(ip), port(port) {}
  bool operator <(const Endpoint &other) const
  {
    if (ip_addr.size() < other.ip_addr.size())
      return true;
    else if (ip_addr.size() > other.ip_addr.size())
      return false;
    else
      return port < other.port;
  }

  IPAddr ip_addr;
  Port port;
};

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
