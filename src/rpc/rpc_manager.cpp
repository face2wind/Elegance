#include "rpc_manager.hpp"
#include "network/network.hpp"

namespace face2wind
{

IRpcRequest::IRpcRequest() : m_data(NULL), m_data_length(0)
{

}

IRpcRequest::IRpcRequest(char *data, int length) : IRpcRequest()
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

RPCManager::RPCManager() : m_handler(NULL), m_network(NULL), m_cur_network_id(0), m_cur_ip_addr(""), m_cur_port(0), m_cur_has_connected(false)
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
  
}

void RPCManager::AsyncListen(Port port, const std::string &key)
{
  
}

void RPCManager::AsyncCall(IRpcRequest *req)
{

}

const char * RPCManager::SyncCall(char * data, int length, int & return_length)
{
	return nullptr;
}

void RPCManager::OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port)
{
	if (local_port == m_cur_port && remote_ip_addr == m_cur_ip_addr)
	{
	if (NULL != m_handler)
		m_handler->OnConnected(remote_ip_addr, remote_port, is_success);

	}
}

void RPCManager::OnAccept(bool is_success, NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port)
{
	if (NULL != m_handler)
		m_handler->OnListened();


	m_cur_network_id = network_id;
}

void RPCManager::OnRecv(NetworkID network_id, const char *data, int length)
{
	if (network_id != m_cur_network_id)
		return;


}

void RPCManager::OnDisconnect(NetworkID network_id)
{
	if (network_id != m_cur_network_id)
		return;

	m_cur_has_connected = false;
}

}

