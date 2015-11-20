#include "rpc_manager.hpp"
#include "network/network.hpp"

namespace face2wind
{

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
      int result_data_len(0);
      char *result_data = m_message_buffer + sizeof(RPCMessageHeader);
			
      int handle_result = (*it)->HandleCall(data + sizeof(RPCMessageHeader), length - sizeof(RPCMessageHeader), result_data, result_data_len);
      if (handle_result != 0)
        continue;

      if (NULL == result_data || result_data_len <= 0)
        continue;

      int total_send_len = result_data_len + sizeof(RPCMessageHeader);
      if (total_send_len >= RPC_SESSION_NETWORK_MESSAGE_MAX_LEN)
        continue;

      RPCMessageHeader *receive_head = (RPCMessageHeader *)data;

      m_send_message_lock.lock();

      RPCMessageHeader *send_head = (RPCMessageHeader *)m_message_buffer;
      send_head->type = RPC_MESSAGE_TYPE_RESPONSE;
      send_head->message_id = receive_head->message_id;
      //memcpy(m_message_buffer + sizeof(RPCMessageHeader), result_data, result_data_len);
      m_network->AsyncSendData(m_network_id, m_message_buffer, total_send_len);

      m_send_message_lock.unlock();
    }
  }
}

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
    DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());
        
    delete req;
    return;
  }

  int request_id = this->GetRequestID();

  m_send_message_lock.lock();

  RPCMessageHeader *head = (RPCMessageHeader *)m_message_buffer;
  head->type = RPC_MESSAGE_TYPE_REQUEST;
  head->message_id = request_id;
  memcpy(m_message_buffer + sizeof(RPCMessageHeader), data, length);

  if (m_network->AsyncSendData(m_network_id, m_message_buffer, total_len))
    m_request_list[request_id] = req;

  m_send_message_lock.unlock();
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
	}
	else
	{
		new_request_id = m_free_request_id_stack.top();
		m_free_request_id_stack.pop();
	}

  return new_request_id;
}

RPCManager::RPCManager(IRpcConnectHandler *handler) : m_handler(handler), m_network(nullptr)
{
  m_network = Network::GetInstance();
  if (nullptr != m_network)
    m_network->AsyncRun();
  m_network->RegistHandler(this);
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
  m_uncheck_key_map[uncheck_key] = key;
  m_network->AsyncConnect(server_ip, port);
}

void RPCManager::AsyncListen(Port port, const std::string &key)
{
  m_listen_port_2_key_map[port] = key;
  m_network->AsyncListen(port);
}

void RPCManager::OnActiveNetwork(NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
{
  std::string session_id = CalculateRPCSessionKey(listen_port, remote_ip_addr, remote_port);
  if (m_session_map.find(session_id) == m_session_map.end())
  {
    m_session_map[session_id].SetData(remote_ip_addr, remote_port, m_network, network_id);
  }

  m_session_id_2_network_id_map[session_id] = network_id;
  m_network_id_2_session_id_map[network_id] = session_id;

  if (nullptr != m_handler)
    m_handler->OnSessionActive(&m_session_map[session_id]);
}

void RPCManager::OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port)
{
	if (!is_success)
	{
		if (nullptr != m_handler)
			m_handler->OnConnectFail(remote_ip_addr, remote_port);
		return;
	}

	std::string uncheck_key = CalculateRPCSessionKey(0, remote_ip_addr, remote_port);
  if (m_uncheck_key_map.find(uncheck_key) != m_uncheck_key_map.end())
  {
    std::string rpc_key = m_uncheck_key_map[uncheck_key];

    if (rpc_key.size() < RPC_MAX_KEY_STRING_LENGTH)
    {
      UncheckRPCInfo info;
      info.network_id = network_id;
      info.local_port = local_port;
      info.remote_ip_addr = remote_ip_addr;
      info.remote_port = remote_port;
      m_uncheck_rpc_info_map[network_id] = info;
      
      static RPCMessageCheckKeyCS rpc_key_check_cs;
      rpc_key_check_cs.key_length = static_cast<short>(rpc_key.size());
      memcpy(rpc_key_check_cs.key_str, rpc_key.c_str(), rpc_key.size());
      rpc_key_check_cs.key_str[rpc_key.size()] = '\0';
      rpc_key_check_cs.key_str[RPC_MAX_KEY_STRING_LENGTH - 1] = '\0';

      int send_len = sizeof(rpc_key_check_cs) - (RPC_MAX_KEY_STRING_LENGTH - rpc_key.size());
      m_network->AsyncSendData(network_id, (char *)&rpc_key_check_cs, send_len);
    }
  }
  //this->OnActiveNetwork(is_success, network_id, local_port, remote_ip_addr, remote_port);
}

void RPCManager::OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
{
	if (!is_success)
	{
		if (nullptr != m_handler)
			m_handler->OnListenFail(listen_port);
		return;
	}
  
  if (m_listen_port_2_key_map.find(listen_port) != m_listen_port_2_key_map.end())
  {
    m_network_id_2_key_map[network_id] = m_listen_port_2_key_map[listen_port];

    if (m_uncheck_rpc_info_map.find(network_id) == m_uncheck_rpc_info_map.end())
    {
      UncheckRPCInfo info;
      info.network_id = network_id;
      info.local_port = listen_port;
      info.remote_ip_addr = remote_ip_addr;
      info.remote_port = remote_port;
      m_uncheck_rpc_info_map[network_id] = info;
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

    //std::cout << "receive key check : "<< receive_key <<" == " << m_network_id_2_key_map[network_id] << std::endl;
    if (receive_key == m_network_id_2_key_map[network_id]) // key match
    {
      static RPCMessageCheckKeyResponseSC check_key_response;
      check_key_response.type = RPC_MESSAGE_TYPE_CHECK_KEY_RESPONSE;
      check_key_response.response = 1;
      m_network->AsyncSendData(network_id, (char *)&check_key_response, sizeof(check_key_response));

      UncheckRPCInfo &uncheck_info = m_uncheck_rpc_info_map[network_id];
      this->OnActiveNetwork(network_id, uncheck_info.local_port, uncheck_info.remote_ip_addr, uncheck_info.remote_port);
    }
    else
    {
      m_network->Disconnect(network_id);
      // disconnect it;
    }
  }
  else  if (RPC_MESSAGE_TYPE_CHECK_KEY_RESPONSE == *rpc_message_type)
  {
    RPCMessageCheckKeyResponseSC *respon_sc = (RPCMessageCheckKeyResponseSC*)data;
    //std::cout << "receive key check result : " << respon_sc->response << std::endl;
    if (1 == respon_sc->response)
    {
      UncheckRPCInfo &uncheck_info = m_uncheck_rpc_info_map[network_id];
      this->OnActiveNetwork(network_id, uncheck_info.local_port, uncheck_info.remote_ip_addr, uncheck_info.remote_port);
    }
    else
    {
      m_network->Disconnect(network_id);
    }
  }
  else
  {
    std::string session_id = m_network_id_2_session_id_map[network_id];
    
    if (m_session_map.find(session_id) != m_session_map.end())
      m_session_map[session_id].OnRecv(data, length);
  }
}

void RPCManager::OnDisconnect(NetworkID network_id)
{
  std::string session_id = m_network_id_2_session_id_map[network_id];
  
  m_session_id_2_network_id_map.erase(session_id);
  m_network_id_2_session_id_map.erase(network_id);

  if (nullptr != m_handler)
    m_handler->OnSessionInactive(&m_session_map[session_id]);
}

}

