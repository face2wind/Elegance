#pragma once

namespace face2wind {

typedef unsigned short Port;
typedef std::string IPAddr;

class ISocketHandler
{
 public:
  ISocketHandler();
  virtual ~ISocketHandler();

  void OnListenResult(Port port, bool success) = 0;
  void OnConnectResult(IPAddr ip, Port port, bool success) = 0;
  
  void OnAccept(IPAddr ip, Port port) = 0;
  void OnConnect(IPAddr ip, Port port) = 0;
  
  void OnRecv(IPAddr ip, Port port, char *data, int length) = 0;
  
  void OnDisconnect(IPAddr ip, Port port) = 0;
};

class Socket
{
 public:
  Socket();
  ~Socket();

  bool Listen(Port port);
  void AsyncListen(Port port);
  
  bool Connect(IPAddr ip, Port port, int timeout_s = 0);
  void AsyncConnect(IPAddr ip, Port port, int timeout_s = 0);

  bool RegistHandler(ISocketHandler *handler);
  bool UnregistHandler(ISocketHandler *handler);
};

}
