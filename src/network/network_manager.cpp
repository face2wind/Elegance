#include "network_manager.hpp"

#include <platform/network/socket_accept.hpp>
#include <platform/network/socket_connect.hpp>

namespace face2wind
{

NetworkManager::NetworkManager() : max_net_id_(0)
{

}
  
NetworkManager::~NetworkManager()
{
  for (Thread *thread : thread_set_)
  {
    delete thread;
  }

  thread_set_.clear();
}

void NetworkManager::RegistHandler(INetworkHandler *handler)
{
  if (handler_list_.find(handler) != handler_list_.end())
    return;

  handler_list_.insert(handler);
}

void NetworkManager::UnregistHandler(INetworkHandler *handler)
{
  if (handler_list_.find(handler) == handler_list_.end())
    return;

  handler_list_.erase(handler);
}

void NetworkManagerListenTask::Run()
{
  NetworkManager *mgr = (NetworkManager*)param_;
  if (NULL != mgr)
    mgr->ListenThread(port);
}

void NetworkManagerConnectTask::Run()
{
  NetworkManager *mgr = (NetworkManager*)param_;
  if (NULL != mgr)
    mgr->ConnectThread(ip, port);
}

void NetworkManager::SyncListen(Port port)
{
  NetworkManagerListenTask *task = new NetworkManagerListenTask();
  task->SetParam(this);
  task->port = port;
  Thread *thread = this->GetFreeThread();
  if (nullptr != thread)
    thread->Run(task);
}

void NetworkManager::SyncConnect(IPAddr ip, Port port)
{
  NetworkManagerConnectTask *task = new NetworkManagerConnectTask();
  task->SetParam(this);
  task->ip = ip;
  task->port = port;
  Thread *thread = this->GetFreeThread();
  if (nullptr != thread)
    thread->Run(task);
}


void NetworkManager::Send(NetworkID net_id, const char *data, int length)
{
  auto endpoint_it = net_id_to_endpoint_map_.find(net_id);
  if (endpoint_it == net_id_to_endpoint_map_.end())
    return;

  auto accept_it = net_id_to_accept_.find(net_id);
  if (accept_it != net_id_to_accept_.end())
    accept_it->second->Write(endpoint_it->second.ip_addr, endpoint_it->second.port, data, length);

  auto connect_it = net_id_to_connect_.find(net_id);
  if (connect_it != net_id_to_connect_.end())
    connect_it->second->Write(data, length);
}

void NetworkManager::Disconnect(NetworkID net_id)
{
  auto endpoint_it = net_id_to_endpoint_map_.find(net_id);
  if (endpoint_it == net_id_to_endpoint_map_.end())
    return;

  auto accept_it = net_id_to_accept_.find(net_id);
  if (accept_it != net_id_to_accept_.end())
    accept_it->second->Disconnect(endpoint_it->second.ip_addr, endpoint_it->second.port);

  auto connect_it = net_id_to_connect_.find(net_id);
  if (connect_it != net_id_to_connect_.end())
    connect_it->second->Disconnect();
}

void NetworkManager::WaitAllThread()
{
  for (Thread *thread : thread_set_)
  {
    if (thread->IsRunning())
    {
      thread->Join();
      delete thread;
    }
  }
  thread_set_.clear();
}

Thread * NetworkManager::GetFreeThread()
{
  for (Thread *thread : thread_set_)
  {
    if (!thread->IsRunning())
      return thread;
  }

  Thread *tmp_thread = new Thread();
  thread_set_.insert(tmp_thread);
  return tmp_thread;
}

NetworkID NetworkManager::GetFreeNetID()
{
  NetworkID tmp_net_id = 0;

  if (!free_net_id_list_.empty())
  {
    tmp_net_id = free_net_id_list_.top();
    free_net_id_list_.pop();
  }
  else
  {
    tmp_net_id = max_net_id_ + 1;
    max_net_id_ = tmp_net_id;
  }

  return tmp_net_id;
}

// all handle functions

void NetworkManager::ListenThread(Port port)
{
  SocketAccept *accept = new SocketAccept();
  accept->ResetHandler(this);
  //accept_list_mutex_.Lock();
  accept_list_.insert(accept);
  //accept_list_mutex_.Unlock();

  bool listen_result = accept->Listen(port);

  if (!listen_result)
  {
    for (auto handler : handler_list_)
      handler->OnListenFail(port);
  }

  accept_list_.erase(accept);
  delete accept;
}

void NetworkManager::ConnectThread(IPAddr ip, Port port)
{
  SocketConnect *connect = new SocketConnect();
  connect->ResetHandler(this);
	
  //accept_list_mutex_.Lock();
  connect_list_.insert(connect);
  //accept_list_mutex_.Unlock();

  bool connect_result = connect->Connect(ip, port);

  if (!connect_result)
  {
    for (auto handler : handler_list_)
      handler->OnConnect(ip, port, false);
  }

  connect_list_.erase(connect);
  delete connect;
}

void NetworkManager::OnAccept(IPAddr ip, Port port)
{
  NetworkID net_id = this->GetFreeNetID();

  for (auto accept : accept_list_)
  {
    if (accept->CheckOnHandle(ip, port))
    {
      net_id_to_accept_[net_id] = accept;

      Endpoint end_point(ip, port);
      net_id_to_endpoint_map_[net_id] = end_point;
      endpoint_to_net_id_map_[end_point] = net_id;
    }
  }

  for (auto handler : handler_list_)
    handler->OnAccept(ip, port, net_id);
}

void NetworkManager::OnConnect(IPAddr ip, Port port)
{
  NetworkID net_id = this->GetFreeNetID();

  for (auto connect : connect_list_)
  {
    if (connect->CheckOnHandle(ip, port))
    {
      net_id_to_connect_[net_id] = connect;

      Endpoint end_point(ip, port);
      net_id_to_endpoint_map_[net_id] = end_point;
      endpoint_to_net_id_map_[end_point] = net_id;
    }
  }

  for (auto handler : handler_list_)
    handler->OnConnect(ip, port, true, net_id);
}

void NetworkManager::OnRecv(IPAddr ip, Port port, char *data, int length)
{
  auto net_id_it = endpoint_to_net_id_map_.find(Endpoint(ip, port));
  if (net_id_it == endpoint_to_net_id_map_.end())
    return;

  for (auto handler : handler_list_)
    handler->OnRecv(net_id_it->second, data, length);
}

void NetworkManager::OnDisconnect(IPAddr ip, Port port)
{
  auto net_id_it = endpoint_to_net_id_map_.find(Endpoint(ip, port));
  if (net_id_it == endpoint_to_net_id_map_.end())
    return;

  net_id_to_endpoint_map_.erase(net_id_it->second);
  endpoint_to_net_id_map_.erase(net_id_it);

  auto accept_it = net_id_to_accept_.find(net_id_it->second);
  if (accept_it != net_id_to_accept_.end())
  {
    net_id_to_accept_.erase(accept_it);
  }

  auto connect_it = net_id_to_connect_.find(net_id_it->second);
  if (connect_it != net_id_to_connect_.end())
  {
    net_id_to_connect_.erase(connect_it);
  }

  for (auto handler : handler_list_)
    handler->OnDisconnect(net_id_it->second);

  free_net_id_list_.push(net_id_it->second);
}


}
