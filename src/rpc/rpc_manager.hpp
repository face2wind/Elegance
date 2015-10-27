#ifndef __RPC_MANAGER_HPP__
#define __RPC_MANAGER_HPP__

#include <string>
#include "network/i_network.hpp"

namespace face2wind
{
class IRpcHandler
{
 public:
  virtual void OnConnected(const std::string &server_ip, Port port, bool success) = 0;
  virtual void OnListened(const std::string &server_ip, Port port, bool success) = 0;
};

class IRpcRequest
{
 public:
  IRpcRequest();
  IRpcRequest(char *data, int length);
  ~IRpcRequest();
  
  void SetData(char *data, int length);
  
  virtual void OnCallBack(char *data, int length) = 0;
};

class RPCManager
{
public:
  RPCManager();
  ~RPCManager();
  
  void AsyncConnect(const std::string &server_ip, Port port, const std::string &key);
  void AsyncListen(Port port, const std::string &key);

  void AsyncCall(IRpcRequest *req);
  void SyncCall(char *data, int length, char *return_data, int &return_length);

 protected:
  IRpcHandler *handler;
};
};

#endif
