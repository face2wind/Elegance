#include "rpc_manager.hpp"
#include "network/network_manager.hpp"
#include "common/debug_message.hpp"

#include <sstream>
#include <cstring>
#include <limits.h>

namespace face2wind
{

void RPCSession::RegisterHandler(IRpcHandler * handler)
{
  if (nullptr != handler && m_handler_list.find(handler) == m_handler_list.end())
  {
    m_handler_list.insert(handler);
  }
}

void RPCSession::AsyncCall(IRpcRequest *req, const char *data, int length)
{
  if (nullptr == req)
    return;

  if (nullptr == data || length <= 0)
  {
    delete req;
    return;
  }

  int total_len = length + sizeof(RPCMessageHeader);
  if (total_len >= RPC_SESSION_NETWORK_MESSAGE_MAX_LEN)
  {
    std::stringstream ss;
    ss << "RPCSession::AsyncCall, total data length is more than "<< RPC_SESSION_NETWORK_MESSAGE_MAX_LEN << ", request give up.";
    DebugMessage::GetInstance().ShowMessage(DebugMessageType::REMOTE_PROCEDURE_CALL, ss.str());
        
    delete req;
    return;
  }

  int request_id = this->GetRequestID();

  m_send_message_lock.Lock();

  RPCMessageHeader *head = (RPCMessageHeader *)m_message_buffer;
  head->type = RPC_MESSAGE_TYPE_REQUEST;
  head->message_id = request_id;
  memcpy(m_message_buffer + sizeof(RPCMessageHeader), data, length);

  m_network_mgr->Send(m_network_id, m_message_buffer, total_len);
  m_request_list[request_id] = req;

  m_send_message_lock.Unlock();
}

const char * RPCSession::SyncCall(const char * data, int length, int & return_length)
{
  return nullptr;
}

int RPCSession::GetRequestID()
{
	int new_request_id(0);

	if (m_free_request_id_stack.empty())
	{
		new_request_id = m_max_request_id ++;
		m_max_request_id %= SHRT_MAX;
	}
	else
	{
		new_request_id = m_free_request_id_stack.top();
		m_free_request_id_stack.pop();
	}

  return new_request_id;
}

void RPCSession::OnRecv(const char *data, int length)
{
	if (nullptr == data || length <= static_cast<int>(sizeof(RPCMessageHeader)))
		return;

	RPCMessageHeader *head = (RPCMessageHeader *)data;
	if (RPC_MESSAGE_TYPE_RESPONSE == head->type)
	{
		std::map<int, IRpcRequest*>::iterator request_it = m_request_list.find(head->message_id);
		if (request_it != m_request_list.end())
		{
			IRpcRequest *request = request_it->second;
			if (NULL != request)
			{
				request->OnCallBack(data + sizeof(RPCMessageHeader), length - sizeof(RPCMessageHeader));
				delete request;
			}

			m_request_list.erase(request_it);
			m_free_request_id_stack.push(head->message_id);
		}
	}
	else if (RPC_MESSAGE_TYPE_REQUEST == head->type)
	{
		for (std::set<IRpcHandler*>::iterator it = m_handler_list.begin(); it != m_handler_list.end(); ++it)
		{
			int result_data_len = RPC_SESSION_NETWORK_MESSAGE_MAX_LEN - sizeof(RPCMessageHeader);
			char *result_data = m_message_buffer + sizeof(RPCMessageHeader);

			m_send_message_lock.Lock();

			do
			{
				int handle_result = (*it)->HandleCall(data + sizeof(RPCMessageHeader), length - sizeof(RPCMessageHeader), result_data, result_data_len);
				if (handle_result != 0)
					break;

				if (NULL == result_data || result_data_len <= 0)
					break;

				int total_send_len = result_data_len + sizeof(RPCMessageHeader);
				if (total_send_len >= RPC_SESSION_NETWORK_MESSAGE_MAX_LEN)
					break;

				RPCMessageHeader *receive_head = (RPCMessageHeader *)data;

				RPCMessageHeader *send_head = (RPCMessageHeader *)m_message_buffer;
				send_head->type = RPC_MESSAGE_TYPE_RESPONSE;
				send_head->message_id = receive_head->message_id;
				//memcpy(m_message_buffer + sizeof(RPCMessageHeader), result_data, result_data_len);
				m_network_mgr->Send(m_network_id, m_message_buffer, total_send_len);
			} while (false);

			m_send_message_lock.Unlock();
		}
	}
}

RPCManager::RPCManager(IRpcConnectHandler *handler, NetworkManager *network_mgr) : handler_(handler), network_mgr_(network_mgr)
{
  if (nullptr != network_mgr_)
    network_mgr_->RegistHandler(this);
}

RPCManager::~RPCManager()
{

}

std::string CalculateRPCSessionKey(Port local_port, IPAddr remote_ip_addr, Port remote_port)
{
  std::stringstream ss;
  ss << local_port << "_" << remote_ip_addr << "_" << remote_port;
  return ss.str();
}

void RPCManager::AsyncConnect(const IPAddr &server_ip, Port port, const std::string &key)
{
  std::string uncheck_key = CalculateRPCSessionKey(0, server_ip, port);
  uncheck_key_map_[uncheck_key] = key;
  network_mgr_->SyncConnect(server_ip, port);
}

void RPCManager::AsyncListen(Port port, const std::string &key)
{
  listen_port_2_key_map_[port] = key;
  network_mgr_->SyncListen(port);
}

void RPCManager::OnActiveNetwork(NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
{
  std::string session_id = CalculateRPCSessionKey(listen_port, remote_ip_addr, remote_port);
  if (session_map_.find(session_id) == session_map_.end())
  {
    session_map_[session_id].SetData(remote_ip_addr, remote_port, listen_port, network_mgr_, network_id);
  }

  session_id_2_network_id_map_[session_id] = network_id;
  network_id_2_session_id_map_[network_id] = session_id;

  if (nullptr != handler_)
    handler_->OnSessionActive(&session_map_[session_id]);
}

//void RPCManager::OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port)
void RPCManager::OnConnect(IPAddr ip, Port port, Port local_port, bool success, NetworkID net_id)
{
	if (!success)
	{
		if (nullptr != handler_)
			handler_->OnConnectFail(ip, port);
		return;
	}

	std::string uncheck_key = CalculateRPCSessionKey(0, ip, port);
  if (uncheck_key_map_.find(uncheck_key) != uncheck_key_map_.end())
  {
    std::string rpc_key = uncheck_key_map_[uncheck_key];

    if (rpc_key.size() < RPC_MAX_KEY_STRING_LENGTH)
    {
      UncheckRPCInfo info;
      info.network_id = net_id;
      info.remote_ip_addr = ip;
      info.remote_port = port;
	  info.local_port = local_port;
      uncheck_rpc_info_map_[net_id] = info;
      
      static RPCMessageCheckKeyCS rpc_key_check_cs;
      rpc_key_check_cs.key_length = static_cast<short>(rpc_key.size());
      memcpy(rpc_key_check_cs.key_str, rpc_key.c_str(), rpc_key.size());
      rpc_key_check_cs.key_str[rpc_key.size()] = '\0';
      rpc_key_check_cs.key_str[RPC_MAX_KEY_STRING_LENGTH - 1] = '\0';

      int send_len = sizeof(rpc_key_check_cs) - (RPC_MAX_KEY_STRING_LENGTH - static_cast<int>(rpc_key.size()));
      network_mgr_->Send(net_id, (char *)&rpc_key_check_cs, send_len);
    }
  }
  //this->OnActiveNetwork(is_success, network_id, local_port, remote_ip_addr, remote_port);
}

void RPCManager::OnListenFail(Port port)
{
	if (nullptr != handler_)
		handler_->OnListenFail(port);
}

void RPCManager::OnAccept(IPAddr remote_ip_addr, Port remote_port, Port local_port, NetworkID net_id)
//void RPCManager::OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
{ 
  if (listen_port_2_key_map_.find(local_port) != listen_port_2_key_map_.end())
  {
    network_id_2_key_map_[net_id] = listen_port_2_key_map_[local_port];

    if (uncheck_rpc_info_map_.find(net_id) == uncheck_rpc_info_map_.end())
    {
      UncheckRPCInfo info;
      info.network_id = net_id;
      info.remote_ip_addr = remote_ip_addr;
      info.remote_port = remote_port;
      uncheck_rpc_info_map_[net_id] = info;
    }
  }
  //this->OnActiveNetwork(is_success, network_id, listen_port, remote_ip_addr, remote_port);
}

void RPCManager::OnRecv(NetworkID network_id, const char *data, int length)
{
  const char *rpc_message_type = data;
  if (RPC_MESSAGE_TYPE_CHECK_KEY == *rpc_message_type)
  {
    RPCMessageCheckKeyCS *check_key_cs = (RPCMessageCheckKeyCS*)data;
    std::string receive_key(check_key_cs->key_str);

	std::stringstream ss;
	ss << "receive key check : " << receive_key << " == " << network_id_2_key_map_[network_id];
	DebugMessage::GetInstance().ShowMessage(DebugMessageType::REMOTE_PROCEDURE_CALL, ss.str());

    if (receive_key == network_id_2_key_map_[network_id]) // key match
    {
      static RPCMessageCheckKeyResponseSC check_key_response;
      check_key_response.type = RPC_MESSAGE_TYPE_CHECK_KEY_RESPONSE;
      check_key_response.response = 1;
      network_mgr_->Send(network_id, (char *)&check_key_response, sizeof(check_key_response));

      UncheckRPCInfo &uncheck_info = uncheck_rpc_info_map_[network_id];
      this->OnActiveNetwork(network_id, uncheck_info.local_port, uncheck_info.remote_ip_addr, uncheck_info.remote_port);
    }
    else
    {
      network_mgr_->Disconnect(network_id);
      // disconnect it;
    }
  }
  else  if (RPC_MESSAGE_TYPE_CHECK_KEY_RESPONSE == *rpc_message_type)
  {
    RPCMessageCheckKeyResponseSC *respon_sc = (RPCMessageCheckKeyResponseSC*)data;

	std::stringstream ss;
	ss << "receive key check result : " << respon_sc->response;
	DebugMessage::GetInstance().ShowMessage(DebugMessageType::REMOTE_PROCEDURE_CALL, ss.str());

    if (1 == respon_sc->response)
    {
      UncheckRPCInfo &uncheck_info = uncheck_rpc_info_map_[network_id];
      this->OnActiveNetwork(network_id, uncheck_info.local_port, uncheck_info.remote_ip_addr, uncheck_info.remote_port);
    }
    else
    {
      network_mgr_->Disconnect(network_id);
    }
  }
  else
  {
    std::string session_id = network_id_2_session_id_map_[network_id];
    
    if (session_map_.find(session_id) != session_map_.end())
      session_map_[session_id].OnRecv(data, length);
  }
}

void RPCManager::OnDisconnect(NetworkID network_id)
{
  std::string session_id = network_id_2_session_id_map_[network_id];
  
  session_id_2_network_id_map_.erase(session_id);
  network_id_2_session_id_map_.erase(network_id);

  if (nullptr != handler_)
    handler_->OnSessionInactive(&session_map_[session_id]);
}

}

