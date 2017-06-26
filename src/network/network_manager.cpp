#include "network_manager.hpp"

#include <platform/network/socket_accept.hpp>
#include <platform/network/socket_connect.hpp>

#include "common/debug_message.hpp"
#include <sstream>

namespace face2wind
{

NetworkManager::NetworkManager() : max_net_id_(0), packager_(nullptr), packer_type_(NetworkPackerType::BEGIN)
{
  packager_ = NetworkPackerFactory::CreatePacker(packer_type_, this);
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

  //  std::stringstream ss;
  //ss << "NetworkManager::RegistHandler(" << handler << ")";
  //DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::RegistHandler()" << fDebugEndl;
}

void NetworkManager::UnregistHandler(INetworkHandler *handler)
{
  if (handler_list_.find(handler) == handler_list_.end())
    return;

  handler_list_.erase(handler);

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::UnregistHandler()" << fDebugEndl;
}

void NetworkManagerListenTask::Run()
{
  NetworkManager *mgr = (NetworkManager*)param_;
  if (nullptr != mgr)
    mgr->ListenThread(port);
}

void NetworkManagerConnectTask::Run()
{
  NetworkManager *mgr = (NetworkManager*)param_;
  if (nullptr != mgr)
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
  net_id_endpoint_lock_.Lock(); 
  auto endpoint_it = net_id_to_endpoint_map_.find(net_id);
  if (endpoint_it == net_id_to_endpoint_map_.end())
  {
    net_id_endpoint_lock_.Unlock();
    return;
  }
  net_id_endpoint_lock_.Unlock();

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::Send net_id = " << net_id << " length = " << length << fDebugEndl;

  if (nullptr != packager_)
    packager_->PackAndSend(net_id, data, length);
}

void NetworkManager::Disconnect(NetworkID net_id)
{
  net_id_endpoint_lock_.Lock();
  auto endpoint_it = net_id_to_endpoint_map_.find(net_id);
  if (endpoint_it == net_id_to_endpoint_map_.end())
  {
    net_id_endpoint_lock_.Lock();
    return;
  }
  net_id_endpoint_lock_.Lock();
  
  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::Disconnect net_id = " << net_id << fDebugEndl;

  auto accept_it = net_id_to_accept_.find(net_id);
  if (accept_it != net_id_to_accept_.end())
    accept_it->second->Disconnect(endpoint_it->second.remote_ip_addr, endpoint_it->second.remote_port);

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
  NetworkID tmp_net_id;

  free_net_id_lock_.Lock();
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
  free_net_id_lock_.Unlock();

  return tmp_net_id;
}

// all handle functions

void NetworkManager::ListenThread(Port port)
{
  SocketAccept *accept = new SocketAccept();
  accept->ResetHandler(this);

  accept_list_mutex_.Lock();
  accept_list_.insert(accept);
  accept_list_mutex_.Unlock();

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::ListenThread port = " << port << fDebugEndl;

  bool listen_result = accept->Listen(port);

  if (!listen_result)
  {
    for (auto handler : handler_list_)
      handler->OnListenFail(port);
  }

  accept_list_mutex_.Lock();
  accept_list_.erase(accept);
  accept_list_mutex_.Unlock();

  delete accept;
}

void NetworkManager::ConnectThread(IPAddr ip, Port port)
{
  SocketConnect *connect = new SocketConnect();
  connect->ResetHandler(this);
	
  connect_list_mutex_.Lock();
  connect_list_.insert(connect);
  connect_list_mutex_.Unlock();

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::ConnectThread remote = " << ip << ":" << port << fDebugEndl;

  bool connect_result = connect->Connect(ip, port);

  if (!connect_result)
  {
    for (auto handler : handler_list_)
      handler->OnConnect(ip, port, connect->GetLocalPort(), false, 0);
  }

  connect_list_mutex_.Lock();
  connect_list_.erase(connect);
  connect_list_mutex_.Unlock();
  delete connect;
}

void NetworkManager::OnAccept(IPAddr remote_ip, Port remote_port, Port local_port)
{
  NetworkID net_id = this->GetFreeNetID();

  accept_list_mutex_.Lock();
  for (auto accept : accept_list_)
  {
    if (accept->CheckOnHandle(remote_ip, remote_port))
    {
      net_id_to_accept_[net_id] = accept;

      Endpoint end_point(remote_ip, remote_port, local_port);

      net_id_endpoint_lock_.Lock();
      net_id_to_endpoint_map_[net_id] = end_point;
      endpoint_to_net_id_map_[end_point] = net_id;
      net_id_endpoint_lock_.Unlock();
    }
  }
  accept_list_mutex_.Unlock();

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::OnAccept remote = " << remote_ip << ":" << remote_port << ", local_port = " << local_port << ", net_id = "<< net_id << fDebugEndl;

  for (auto handler : handler_list_)
    handler->OnAccept(remote_ip, remote_port, local_port, net_id);
}

void NetworkManager::OnConnect(IPAddr remote_ip, Port remote_port, Port local_port)
{
  NetworkID net_id = this->GetFreeNetID();

  connect_list_mutex_.Lock();
  for (auto connect : connect_list_)
  {
    if (connect->CheckOnHandle(remote_ip, remote_port))
    {
      net_id_to_connect_[net_id] = connect;

      Endpoint end_point(remote_ip, remote_port, local_port);

      net_id_endpoint_lock_.Lock();
      net_id_to_endpoint_map_[net_id] = end_point;
      endpoint_to_net_id_map_[end_point] = net_id;
      net_id_endpoint_lock_.Unlock();
    }
  }
  connect_list_mutex_.Unlock();

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::OnConnect remote = " << remote_ip << ":" << remote_port << ", local_port = " << local_port << ", net_id = " << net_id << fDebugEndl;

  for (auto handler : handler_list_)
    handler->OnConnect(remote_ip, remote_port, local_port, true, net_id);
}

void NetworkManager::OnRecv(IPAddr ip, Port port, Port local_port, char *data, int length)
{
  net_id_endpoint_lock_.Lock();
  auto net_id_it = endpoint_to_net_id_map_.find(Endpoint(ip, port, local_port));
  if (net_id_it == endpoint_to_net_id_map_.end())
  {
    net_id_endpoint_lock_.Unlock();
    return;
  }
  net_id_endpoint_lock_.Unlock();

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::OnRecv " << ip << ":"<<port<<", data length = " << length << fDebugEndl;

  if (nullptr != packager_)
    packager_->UnPack(net_id_it->second, data, length);
}

void NetworkManager::OnDisconnect(IPAddr ip, Port port, Port local_port)
{
  net_id_endpoint_lock_.Lock();

  auto net_id_it = endpoint_to_net_id_map_.find(Endpoint(ip, port, local_port));
  if (net_id_it == endpoint_to_net_id_map_.end())
  {
    net_id_endpoint_lock_.Unlock();
    return;
  }

  NetworkID net_id = net_id_it->second;

  net_id_to_endpoint_map_.erase(net_id);
  endpoint_to_net_id_map_.erase(net_id_it);

  net_id_endpoint_lock_.Unlock();
  
  auto accept_it = net_id_to_accept_.find(net_id);
  if (accept_it != net_id_to_accept_.end())
  {
    net_id_to_accept_.erase(accept_it);
  }

  auto connect_it = net_id_to_connect_.find(net_id);
  if (connect_it != net_id_to_connect_.end())
  {
    net_id_to_connect_.erase(connect_it);
  }

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::OnDisconnect net_id = " << net_id << fDebugEndl;

  for (auto handler : handler_list_)
    handler->OnDisconnect(net_id);

  free_net_id_list_.push(net_id);
}

void NetworkManager::SendRaw(NetworkID net_id, const char *data, int length)
{
  net_id_endpoint_lock_.Lock();
  
  auto endpoint_it = net_id_to_endpoint_map_.find(net_id);
  if (endpoint_it == net_id_to_endpoint_map_.end())
  {
    net_id_endpoint_lock_.Unlock();
    return;
  }
  net_id_endpoint_lock_.Unlock(); 
	
  auto accept_it = net_id_to_accept_.find(net_id);
  if (accept_it != net_id_to_accept_.end())
    accept_it->second->Write(endpoint_it->second.remote_ip_addr, endpoint_it->second.remote_port, data, length);

  auto connect_it = net_id_to_connect_.find(net_id);
  if (connect_it != net_id_to_connect_.end())
    connect_it->second->Write(data, length);
}

void NetworkManager::OnRecvPackage(NetworkID net_id, char *data, int length)
{
  if (net_id <= 0 || nullptr == data || length <= 0)
    return;

  fDebugWithHead(DebugMessageType::BASE_NETWORK) << "NetworkManager::OnRecvPackage net_id = " << net_id << ", data length = " << length << fDebugEndl;

  for (auto handler : handler_list_)
    handler->OnRecv(net_id, data, length);
}


}
