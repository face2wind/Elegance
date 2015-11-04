#ifndef __RPC_MANAGER_HPP__
#define __RPC_MANAGER_HPP__

#include <string>
#include "network/i_network.hpp"

namespace face2wind
{

const static int RPC_MAX_DATA_LENGTH = 2048;

class Network;

class IRpcHandler
{
 public:
  virtual void OnConnected(const IPAddr &server_ip, Port port, bool success) = 0;
  virtual void OnListened(const IPAddr &server_ip, Port port, bool success) = 0;
};

class IRpcRequest
{
 public:
  IRpcRequest();
  IRpcRequest(char *data, int length);
  ~IRpcRequest();
  
  void SetData(const char *data, int length);
  
  virtual void OnCallBack(char *data, int length) = 0;

	private:
		char *m_data;
		int m_data_length;
};

class RPCManager : public INetworkHandler
{
public:
  RPCManager();
  ~RPCManager();
  
  void AsyncConnect(const IPAddr &server_ip, Port port, const std::string &key);
  void AsyncListen(Port port, const std::string &key);

  void AsyncCall(IRpcRequest *req);
  const char * SyncCall(char *data, int length, int &return_length);

	friend class Network;

protected:
	virtual void OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port);
	virtual void OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port);
	virtual void OnRecv(NetworkID network_id, const char *data, int length);
	virtual void OnDisconnect(NetworkID network_id);

protected:
	IRpcHandler *m_handler;
	Network *m_network;

	 NetworkID m_cur_network_id;
	 IPAddr m_cur_ip_addr;
	 Port m_cur_port;

	 bool m_cur_has_connected;
};
};

#endif
