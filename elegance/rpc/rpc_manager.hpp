#ifndef __RPC_MANAGER_HPP__
#define __RPC_MANAGER_HPP__

#include <map>
#include <set>
#include <stack>

#include "elegance/platform/network/socket_def.hpp"
#include "elegance/platform/thread/mutex.hpp"
#include "elegance/network/network_manager.hpp"

namespace face2wind
{

static const int RPC_SESSION_NETWORK_MESSAGE_MAX_LEN = 10240;
const static int RPC_MAX_KEY_STRING_LENGTH = 99;

class RPCSession;
class RPCManager;

#pragma pack(push, 4)

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

#pragma pack(pop)
  
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
  RPCSession() : network_mgr_(nullptr), network_id_(0), cur_has_connected_(false), next_request_id_(0) {}
  virtual ~RPCSession() {}

  const Endpoint &GetEndpoint() { return endpoint_; }

  void RegisterHandler(IRpcHandler *handler);

  void AsyncCall(IRpcRequest *req, const char *data, int length);
  const char * SyncCall(const char *data, int length, int &return_length);

  friend class RPCManager;

 protected:
  void OnRecv(const char *data, int length);

  void SetData(const IPAddr &remote_ip, Port remote_port, Port local_port, NetworkManager *network, NetworkID network_id)
  {
	  endpoint_.remote_ip_addr = remote_ip;
	  endpoint_.remote_port = remote_port;
	  endpoint_.local_port = local_port;

    network_mgr_ = network;
    network_id_ = network_id;
  }

  int GetRequestID();

 protected:
	 Endpoint endpoint_;
  NetworkManager *network_mgr_;
  NetworkID network_id_;
  bool cur_has_connected_;

  Mutex send_message_lock_;
  char message_buffer_[RPC_SESSION_NETWORK_MESSAGE_MAX_LEN];

  std::set<IRpcHandler*> handler_list_;

  Mutex request_lock_;
  std::map<int, IRpcRequest*> request_list_;

  Mutex free_request_id_lock_;
	int next_request_id_;
	std::stack<int> free_request_id_stack_;
};

class IRpcConnectHandler
{
 public:
	 IRpcConnectHandler() {}
	 virtual ~IRpcConnectHandler() {}

	 virtual void OnListenFail(Port listen_port) = 0;
	 virtual void OnConnectFail(IPAddr remote_ip_addr, Port remote_port) = 0;

  virtual void OnSessionActive(RPCSession *session) = 0;
  virtual void OnSessionInactive(RPCSession *session) = 0;
};

typedef std::string RPCSessionID;

class RPCManager : public INetworkHandler
{
  struct UncheckRPCInfo
  {
    UncheckRPCInfo() : network_id(0), remote_ip_addr(""), remote_port(0), local_port(0) {}
    
    NetworkID network_id;
	IPAddr remote_ip_addr;
	Port remote_port;
	Port local_port;
  };
  
 public:
  RPCManager(IRpcConnectHandler *handler, NetworkManager *network_mgr);
  ~RPCManager();
  
  void AsyncConnect(const IPAddr &server_ip, Port port, const std::string &key);
  void AsyncListen(Port port, const std::string &key);

  friend class NetworkManager;
  
 protected:
  void OnActiveNetwork(NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port);

  virtual void OnListenFail(Port port);

  virtual void OnAccept(IPAddr ip, Port port, Port local_port, NetworkID net_id = 0);
  virtual void OnConnect(IPAddr ip, Port port, Port local_port, bool success, NetworkID net_id);

  virtual void OnRecv(NetworkID network_id, const char *data, int length);
  virtual void OnDisconnect(NetworkID network_id);

 protected:
  IRpcConnectHandler *handler_;
  NetworkManager *network_mgr_;

  std::map<RPCSessionID, NetworkID> session_id_2_network_id_map_;
  std::map<NetworkID, RPCSessionID> network_id_2_session_id_map_;

  std::map<RPCSessionID, RPCSession> session_map_;

  std::map<std::string, std::string> uncheck_key_map_;
  std::map<int, UncheckRPCInfo> uncheck_rpc_info_map_;
  
  std::map<int, std::string> listen_port_2_key_map_;
  std::map<int, std::string> network_id_2_key_map_;
};

}

#endif
