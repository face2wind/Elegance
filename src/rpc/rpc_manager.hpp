#ifndef __RPC_MANAGER_HPP__
#define __RPC_MANAGER_HPP__

#include <string>
#include <map>

#include "network/i_network.hpp"

namespace face2wind
{

const static int RPC_MAX_DATA_LENGTH = 2048;

class Network;

class IRpcRequest
{
 public:
  IRpcRequest();
  IRpcRequest(const char *data, int length);
  ~IRpcRequest();
  
  void SetData(const char *data, int length);
  
  virtual void OnCallBack(char *data, int length) = 0;

	private:
		char *m_data;
		int m_data_length;
};

class RPCSession
{
public:
	RPCSession() : m_remote_ip(""), m_remote_port(0), m_cur_has_connected(false) {}
	RPCSession(const IPAddr &remote_ip, Port remote_port) : m_remote_ip(remote_ip), m_remote_port(remote_port), m_cur_has_connected(false) {}
	virtual ~RPCSession() {}

	const IPAddr &GetRemoteIp() { return m_remote_ip; }
	const Port &GetRemotePort() { return m_remote_port; }

	void OnRecv(const char *data, int length);
	void AsyncCall(IRpcRequest *req);
	const char * SyncCall(char *data, int length, int &return_length);

protected:
	IPAddr m_remote_ip;
	Port m_remote_port;
	bool m_cur_has_connected;
};

class IRpcHandler
{
public:
	virtual void OnSessionActive(RPCSession *session) = 0;
};

typedef std::string RPCSessionID;

class RPCManager : public INetworkHandler
{
public:
  RPCManager(IRpcHandler *handler);
  ~RPCManager();
  
  void AsyncConnect(const IPAddr &server_ip, Port port, const std::string &key);
  void AsyncListen(Port port, const std::string &key);

	friend class Network;

protected:
	void OnActiveNetwork(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port);

	virtual void OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port);
	virtual void OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port);
	virtual void OnRecv(NetworkID network_id, const char *data, int length);
	virtual void OnDisconnect(NetworkID network_id);

protected:
	IRpcHandler *m_handler;
	Network *m_network;

	std::map<RPCSessionID, NetworkID> m_session_id_2_network_id_map;
	std::map<NetworkID, RPCSessionID> m_network_id_2_session_id_map;

	std::map<RPCSessionID, RPCSession> m_session_map;
};
};

#endif
