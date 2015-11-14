#ifndef __RPC_MANAGER_HPP__
#define __RPC_MANAGER_HPP__

#include <string>
#include <map>
#include <set>
#include <boost/thread/mutex.hpp>

#include "network/i_network.hpp"

namespace face2wind
{

static const int RPC_SESSION_NETWORK_MESSAGE_MAX_LEN = 10240;
const static int RPC_MAX_KEY_STRING_LENGTH = 99;

class Network;
class RPCSession;
class RPCManager;

enum RPCMessageType
{
  RPC_MESSAGE_TYPE_CHECK_KEY = 0,
  RPC_MESSAGE_TYPE_CHECK_KEY_RESPONSE,
  RPC_MESSAGE_TYPE_REQUEST,
  RPC_MESSAGE_TYPE_RESPONSE,
};

class RPCMessageHeader
{
 public:
  char type;
  char reserve_ch;
  short message_id;
};

struct RPCMessageCheckKeyCS
{
  RPCMessageCheckKeyCS() : type(RPC_MESSAGE_TYPE_CHECK_KEY), key_length(0) {}
  
  char type;
  short key_length;
  char key_str[RPC_MAX_KEY_STRING_LENGTH];
};

struct RPCMessageCheckKeyResponseSC
{
  RPCMessageCheckKeyResponseSC() : type(RPC_MESSAGE_TYPE_CHECK_KEY_RESPONSE), response(0) {}
  
  char type;
  char response;
};
  
class IRpcRequest
{
 public:
  IRpcRequest() {}
  virtual ~IRpcRequest() {}
  
  virtual void OnCallBack(const char *data, int length) = 0;
};

class IRpcHandler
{
 public:
  IRpcHandler() {}
  virtual ~IRpcHandler() {}
  
  virtual int HandleCall(const char *data, int length, char *return_data, int &return_length) = 0;
};

class RPCSession
{
 public:
  RPCSession() : m_remote_ip(""), m_remote_port(0), m_network(nullptr), m_network_id(0), m_cur_has_connected(false) {}
  RPCSession(const IPAddr &remote_ip, Port remote_port, Network *network, NetworkID network_id)
      : m_remote_ip(remote_ip), m_remote_port(remote_port), m_network(network), m_network_id(network_id), m_cur_has_connected(false) {}
  virtual ~RPCSession() {}

  const IPAddr &GetRemoteIp() { return m_remote_ip; }
  const Port &GetRemotePort() { return m_remote_port; }

  void OnRecv(const char *data, int length);

  void RegisterHandler(IRpcHandler *handler);

  void AsyncCall(IRpcRequest *req, const char *data, int length);
  const char * SyncCall(const char *data, int length, int &return_length);

  friend class RPCManager;

 protected:
  void SetData(const IPAddr &remote_ip, Port remote_port, Network *network, NetworkID network_id)
  {
    m_remote_ip = remote_ip;
    m_remote_port = remote_port;
    m_network = network;
    m_network_id = network_id;
  }

  int GetRequestID();

 protected:
  IPAddr m_remote_ip;
  Port m_remote_port;
  Network *m_network;
  NetworkID m_network_id;
  bool m_cur_has_connected;

  char m_message_buffer[RPC_SESSION_NETWORK_MESSAGE_MAX_LEN];
	
  std::set<IRpcHandler*> m_handler_list;
  std::map<int, IRpcRequest*> m_request_list;
  boost::mutex m_send_message_lock;

};

class IRpcConnectHandler
{
 public:
  virtual void OnSessionActive(RPCSession *session) = 0;
  virtual void OnSessionInactive(RPCSession *session) = 0;
};

typedef std::string RPCSessionID;

class RPCManager : public INetworkHandler
{
  struct UncheckRPCInfo
  {
    UncheckRPCInfo() : network_id(0), local_port(0), remote_ip_addr(""), remote_port(0) {}
    
    NetworkID network_id;
    Port local_port;
    IPAddr remote_ip_addr;
    Port remote_port;    
  };
  
 public:
  RPCManager(IRpcConnectHandler *handler);
  ~RPCManager();
  
  void AsyncConnect(const IPAddr &server_ip, Port port, const std::string &key);
  void AsyncListen(Port port, const std::string &key);

  friend class Network;

 protected:
  void OnActiveNetwork(NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port);
  
  virtual void OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port);
  virtual void OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port);
  virtual void OnRecv(NetworkID network_id, const char *data, int length);
  virtual void OnDisconnect(NetworkID network_id);

 protected:
  IRpcConnectHandler *m_handler;
  Network *m_network;

  std::map<RPCSessionID, NetworkID> m_session_id_2_network_id_map;
  std::map<NetworkID, RPCSessionID> m_network_id_2_session_id_map;

  std::map<RPCSessionID, RPCSession> m_session_map;

  std::map<std::string, std::string> m_uncheck_key_map;
  std::map<int, UncheckRPCInfo> m_uncheck_rpc_info_map;
  
  std::map<int, std::string> m_listen_port_2_key_map;
  std::map<int, std::string> m_network_id_2_key_map;
};
};

#endif
