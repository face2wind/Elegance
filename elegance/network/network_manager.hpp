#pragma once

#include <set>
#include <map>
#include <stack>

#include <elegance/platform/network/socket_def.hpp>
#include <elegance/platform/thread/thread_pool.hpp>
#include <elegance/network/network_packer_factory.hpp>
#include <queue>
#include "elegance/memory/mempool.hpp"

namespace face2wind
{

class SocketAccept;
class SocketConnect;
class NetworkManager;

class INetworkHandler
{
 public:
  INetworkHandler() {}
  virtual ~INetworkHandler() {}

  virtual void OnListenFail(Port port) = 0;
  virtual void OnAccept(IPAddr ip, Port port, Port local_port, NetworkID net_id) = 0;
  virtual void OnConnect(IPAddr ip, Port port, Port local_port, bool success, NetworkID net_id) = 0;
  
  virtual void OnRecv(NetworkID net_id, const char *data, int length) = 0;
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

struct NetworkHandleTask
{
	enum class TaskType
	{
		ON_CONNECT = 0,
		ON_CONNECT_FAIL,
		ON_LISTEN_FAIL,
		ON_ACCEPT,
		ON_DISCONNECT,
		ON_RECV,
	};

	NetworkHandleTask() : task_type(), net_id(0), remote_ip(""), remote_port(0), local_port(0), length(0) {}

	TaskType task_type;
	NetworkID net_id;
	IPAddr remote_ip;
	Port remote_port;
	Port local_port;
	char data[MAX_SOCKET_MSG_BUFF_LENGTH];
	int length;
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
  friend class INetworkPackager;

  void RegistHandler(INetworkHandler *handler);
  void UnregistHandler(INetworkHandler *handler);
  
  void SyncListen(Port port);
  void SyncConnect(IPAddr ip, Port port);
  
  void Send(NetworkID net_id, const char *data, int length);
  void Disconnect(NetworkID net_id);

  void SetSingleThreadHandleMode(bool open) { single_thread_handle_mode_ = open; }
  void HandleNetTask();

  void WaitAllThread();
  void SyncRunning(); // ͬ�����У��˺������������̣�������ѭ���������ô˺������Զ��л������̴߳���ģʽ����֤���ж�INetworkHandler�Ļص�����ͬһ�߳��
  
 protected:
  Thread * GetFreeThread();
  NetworkID GetFreeNetID();

  void ListenThread(Port port);
  void ConnectThread(IPAddr ip, Port port);

  virtual void OnAccept(IPAddr remote_ip, Port remote_port, Port local_port);
  virtual void OnConnect(IPAddr remote_ip, Port remote_port, Port local_port);
  virtual void OnRecv(IPAddr ip, Port port, Port local_port, char *data, int length);
  virtual void OnDisconnect(IPAddr ip, Port port, Port local_port);

  virtual void SendRaw(NetworkID net_id, const char *data, int length);
  virtual void OnRecvPackage(NetworkID net_id, char *data, int length);

 protected:
  std::set<Thread *> thread_set_;

  std::set<INetworkHandler *> handler_list_;
  
  std::set<SocketAccept*> accept_list_;
  std::set<SocketConnect*> connect_list_;

  Mutex free_net_id_lock_;
  std::stack<NetworkID> free_net_id_list_;
  int max_net_id_;

  Mutex net_id_endpoint_lock_;
  std::map<NetworkID, Endpoint> net_id_to_endpoint_map_;
  std::map<Endpoint, NetworkID> endpoint_to_net_id_map_;

  std::map<NetworkID, SocketAccept*> net_id_to_accept_;
  std::map<NetworkID, SocketConnect*> net_id_to_connect_;

  Mutex accept_list_mutex_;
  Mutex connect_list_mutex_;

  INetworkPackager *packager_;
  NetworkPackerType packer_type_;

  bool single_thread_handle_mode_; // ���̴߳���ص�ģʽ
  Mutex handle_task_mutex_;
  std::queue<NetworkHandleTask*> handle_task_queue_; // ���߳��������
  MemoryPool network_task_memory_pool_;
};


}

