#include "rpc_manager.hpp"
#include "network/network.hpp"

namespace face2wind
{

IRpcRequest::IRpcRequest() : m_data(NULL), m_data_length(0)
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
	if (NULL == data || length <= 0 || length > RPC_MAX_DATA_LENGTH)
		return;

	if (NULL != m_data && m_data_length > 0)
		delete [] m_data;

	m_data = new char[length];
	m_data_length = length;

	memcpy(m_data, data, m_data_length);
}

void RPCSession::OnRecv(const char *data, int length)
{

}

void RPCSession::AsyncCall(IRpcRequest *req)
{

}

const char * RPCSession::SyncCall(char * data, int length, int & return_length)
{
	return nullptr;
}

RPCManager::RPCManager(IRpcHandler *handler) : m_handler(handler), m_network(NULL)
{
	m_network = Network::GetInstance();
	if (NULL != m_network)
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
		m_session_map[session_id] = RPCSession(remote_ip_addr, remote_port);
	}

	m_session_id_2_network_id_map[session_id] = network_id;
	m_network_id_2_session_id_map[network_id] = session_id;

	if (NULL != m_handler)
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

