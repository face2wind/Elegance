#pragma once

#include <set>
#include <platform/network/socket_def.hpp>

namespace face2wind
{

class SocketAccept;
class SocketConnect;

typedef int NetworkID;

class INetworkHandler
{
public:
  virtual void OnAccept(IPAddr ip, Port port, bool success, NetworkID net_id = 0) = 0;
  virtual void OnConnect(IPAddr ip, Port port, bool success, NetworkID net_id = 0) = 0;
  
  virtual void OnRecv(NetworkID net_id, char *data, int length) = 0;
  virtual void OnDisconnect(NetworkID net_id) = 0;
};

class NetworkManager : public ISocketHandler
{
 public:
  NetworkManager();
  ~NetworkManager();
  
  friend class SocketAccept;
  friend class SocketConnect;
  
  void SyncListen(Port port);
  void SyncConnect(IPAddr ip, Port port);
  
  void Send(NetworkID net_id, char *data, int length);
  void Disconnect(NetworkID net_id);
  
protected:
  virtual void OnAccept(IPAddr ip, Port port) = 0;
  virtual void OnConnect(IPAddr ip, Port port) = 0;
  virtual void OnRecv(IPAddr ip, Port port, char *data, int length) = 0;
  virtual void OnDisconnect(IPAddr ip, Port port) = 0;

 private:
  std::set<SocketAccept*> accept_list_;
  std::set<SocketConnect*> connect_list_;
};


}

