#pragma once

#include <set>
#include <map>
#include <stack>

#include <platform/network/socket_def.hpp>

#include <platform/thread/mutex.hpp>
#include <platform/thread/thread_pool.hpp>

namespace face2wind
{

class SocketAccept;
class SocketConnect;

typedef int NetworkID;

class INetworkHandler
{
public:
  virtual void OnListenFail(Port port) = 0;
  virtual void OnAccept(IPAddr ip, Port port, NetworkID net_id = 0) = 0;
  virtual void OnConnect(IPAddr ip, Port port, bool success, NetworkID net_id = 0) = 0;
  
  virtual void OnRecv(NetworkID net_id, char *data, int length) = 0;
  virtual void OnDisconnect(NetworkID net_id) = 0;
};

class NetworkManagerListenTask : public IThreadTask
{
public:
	NetworkManagerListenTask() : port(0) {}

	Port port;

	virtual void Run();
};

class NetworkManagerConnectTask : public IThreadTask
{
public:
	NetworkManagerConnectTask() : ip(""), port(0) {}

	IPAddr ip;
	Port port;

	virtual void Run();
};

class NetworkManager : public ISocketHandler
{
 public:
  NetworkManager();
  ~NetworkManager();
  
  friend class SocketAccept;
  friend class SocketConnect;
  friend class NetworkManagerListenTask;
  friend class NetworkManagerConnectTask;

  void RegistHandler(INetworkHandler *handler);
  void UnregistHandler(INetworkHandler *handler);
  
  void SyncListen(Port port);
  void SyncConnect(IPAddr ip, Port port);
  
  void Send(NetworkID net_id, char *data, int length);
  void Disconnect(NetworkID net_id);

  void WaitAllThread();
  
protected:
  Thread * GetFreeThread();
  NetworkID GetFreeNetID();

  void ListenThread(Port port);
  void ConnectThread(IPAddr ip, Port port);

  virtual void OnAccept(IPAddr ip, Port port);
  virtual void OnConnect(IPAddr ip, Port port);
  virtual void OnRecv(IPAddr ip, Port port, char *data, int length);
  virtual void OnDisconnect(IPAddr ip, Port port);

 private:
  std::set<Thread *> thread_set_;

  std::set<INetworkHandler *> handler_list_;
  
  std::set<SocketAccept*> accept_list_;
  std::set<SocketConnect*> connect_list_;

  std::stack<NetworkID> free_net_id_list_;
  int max_net_id_;

  std::map<NetworkID, Endpoint> net_id_to_endpoint_map_;
  std::map<Endpoint, NetworkID> endpoint_to_net_id_map_;

  std::map<NetworkID, SocketAccept*> net_id_to_accept_;
  std::map<NetworkID, SocketConnect*> net_id_to_connect_;

  Mutex accept_list_mutex_;
  Mutex connect_list_mutex_;
};


}

