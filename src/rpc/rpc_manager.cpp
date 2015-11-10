#include "rpc_manager.hpp"
#include "network/network.hpp"

namespace face2wind
{

IRpcRequest::IRpcRequest() : m_data(nullptr), m_data_length(0)
{

}

IRpcRequest::IRpcRequest(const char *data, int length) : IRpcRequest()
{
	this->SetData(data, length);
}

IRpcRequest::~IRpcRequest()
{

}

void IRpcRequest::SetData(const char * data, int length)
{
	if (nullptr == data || length <= 0 || length > RPC_MAX_DATA_LENGTH)
		return;

	std::cout << "11111ssss11111" << std::endl;
	if (nullptr != m_data && m_data_length > 0)
		delete [] m_data;

	m_data = new char[length];
	m_data_length = length;

	std::cout << "1111111jjjjj111" << m_data_length<<std::endl;
	memcpy(m_data, data, m_data_length);
	std::cout << "111ggggg1111111" << std::endl;
}

void RPCSession::OnRecv(const char *data, int length)
{
	if (nullptr == data || length <= sizeof(RPCMessageHeader))
		return;

	RPCMessageHeader *head = (RPCMessageHeader *)data;
	if (RPC_MESSAGE_TYPE_RESPONSE == head->type)
	{
		std::map<int, IRpcRequest*>::iterator request_it = m_request_list.find(head->message_id);
		if (request_it != m_request_list.end())
		{
			IRpcRequest *request = request_it->second;
			if (NULL != request)
				request->OnCallBack(data + sizeof(RPCMessageHeader), length - sizeof(RPCMessageHeader));

			m_request_list.erase(request_it);
		}
	}
	else if (RPC_MESSAGE_TYPE_REQUEST == head->type)
	{
		for (std::set<IRpcHandler*>::iterator it = m_handler_list.begin(); it != m_handler_list.end(); ++it)
		{
			int result_data_len(0);
			char *result_data = m_message_buffer + sizeof(RPCMessageHeader);
			
			if ((*it)->HandleCall(data, length, result_data, result_data_len) != 0)
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
	if (nullptr != handler && m_handler_list.find(handler) != m_handler_list.end())
	{
		m_handler_list.insert(handler);
	}
}

void RPCSession::AsyncCall(IRpcRequest *req)
{
	if (nullptr == req || nullptr == req->m_data || req->m_data_length <= 0)
		return;

	std::cout << "1111111111" << std::endl;
	int total_len = req->m_data_length + sizeof(RPCMessageHeader);
	if (total_len >= RPC_SESSION_NETWORK_MESSAGE_MAX_LEN)
		return;

	int request_id = this->GetRequestID();

	std::cout << "222222222" << std::endl;
	m_send_message_lock.lock();

	RPCMessageHeader *head = (RPCMessageHeader *)m_message_buffer;
	head->type = RPC_MESSAGE_TYPE_REQUEST;
	head->message_id = request_id;
	memcpy(m_message_buffer + sizeof(RPCMessageHeader), req->m_data, req->m_data_length);

	std::cout << "3333333333" << std::endl;
	if (m_network->AsyncSendData(m_network_id, m_message_buffer, total_len))
		m_request_list[request_id] = req;

	m_send_message_lock.unlock();
	std::cout << "4444444444" << std::endl;
}

const char * RPCSession::SyncCall(const char * data, int length, int & return_length)
{
	return nullptr;
}

int RPCSession::GetRequestID()
{
	return 0;
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

void RPCManager::AsyncConnect(const IPAddr &server_ip, Port port, const std::string &key)
{
	m_network->AsyncConnect(server_ip, port);
}

void RPCManager::AsyncListen(Port port, const std::string &key)
{
	m_network->AsyncListen(port);
}

std::string CalculateRPCSessionKey(Port local_port, IPAddr remote_ip_addr, Port remote_port)
{
	std::stringstream ss;
	ss << local_port << "_" << remote_ip_addr << "_" << remote_port;
	return ss.str();
}

void RPCManager::OnActiveNetwork(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
{
	if (false == is_success)
		return;

	std::string session_id = CalculateRPCSessionKey(listen_port, remote_ip_addr, remote_port);
	if (m_session_map.find(session_id) != m_session_map.end())
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
	this->OnActiveNetwork(is_success, network_id, local_port, remote_ip_addr, remote_port);
}

void RPCManager::OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
{
	this->OnActiveNetwork(is_success, network_id, listen_port, remote_ip_addr, remote_port);
}

void RPCManager::OnRecv(NetworkID network_id, const char *data, int length)
{
	std::string session_id = m_network_id_2_session_id_map[network_id];

	if (m_session_map.find(session_id) != m_session_map.end())
		m_session_map[session_id].OnRecv(data, length);
}

void RPCManager::OnDisconnect(NetworkID network_id)
{
	std::string session_id = m_network_id_2_session_id_map[network_id];
	
	m_session_id_2_network_id_map.erase(session_id);
	m_network_id_2_session_id_map.erase(network_id);
}

}

