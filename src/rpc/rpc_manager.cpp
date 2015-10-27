#include "rpc_manager.hpp"

namespace face2wind
{


IRpcRequest::IRpcRequest()
{

}

IRpcRequest::IRpcRequest(char *data, int length)
{

}

IRpcRequest::~IRpcRequest()
{

}

RPCManager::RPCManager() : handler(NULL)
{
}

RPCManager::~RPCManager()
{

}

void RPCManager::AsyncConnect(const std::string & server_ip, Port port, const std::string &key)
{
  
}

void RPCManager::AsyncListen(Port port, const std::string &key)
{
  
}

void RPCManager::AsyncCall(IRpcRequest *req)
{

}

}

