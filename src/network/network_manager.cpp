#include "network/network_manager.hpp"
#include "network/connect_session.hpp"
#include "network/accept_session.hpp"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <sstream>

namespace face2wind
{

	NetworkManager::NetworkManager() : io_service_running(false), m_cur_max_netid(0)
	{
	}

	NetworkManager::~NetworkManager()
	{
		for (std::vector<AcceptSession*>::iterator it = m_accept_session_list.begin(); it != m_accept_session_list.end(); ++ it)
			delete *it;
		m_accept_session_list.clear();

		for (std::vector<ConnectSession*>::iterator it = m_connect_session_list.begin(); it != m_connect_session_list.end(); ++ it)
			delete *it;
		m_connect_session_list.clear();
	}

	NetworkManager *NetworkManager::GetInstance()
	{
		static NetworkManager instance;
		return &instance;
	}

	bool NetworkManager::RegistHandler(INetworkHandler *handler)
	{
		return true;
	}

	bool NetworkManager::AsyncListen(int port)
	{
		tcp::endpoint endpoint(tcp::v4(), port);
		AcceptSession *session = new AcceptSession(m_io_service, endpoint);
		session->m_network_mgr = this;
		if (io_service_running)
			m_io_service.post(boost::bind(&AcceptSession::AsyncListen, session));
		else
			session->AsyncListen();
		m_accept_session_list.push_back(session);
		return true;
	}

	bool NetworkManager::AsyncConnect(const std::string &host, Port port)
	{
		std::stringstream strStream;  
		strStream << port;  
		std::string port_str = strStream.str();  

		//using boost::asio::ip::tcp;
		tcp::resolver resolver(m_io_service);
		tcp::resolver::query query(host, port_str);
		tcp::resolver::iterator iterator = resolver.resolve(query);

		ConnectSession *session = new ConnectSession(m_io_service, iterator);
		session->m_network_mgr = this;
		if (io_service_running)
			m_io_service.post(boost::bind(&ConnectSession::AsyncConnect, session));
		else
			session->AsyncConnect();
		m_connect_session_list.push_back(session);
		return true;
	}

	bool NetworkManager::AsyncSendData(NetworkID network_id, char *data, int length)
	{
		if (0 >= length)
			return false;
		if (m_network_id_socket_map.count(network_id) <= 0)
			return false;


		if (io_service_running)
			m_io_service.post(boost::bind(&NetworkManager::DoAsyncSendData, this, network_id, data, length));
		else
			this->DoAsyncSendData(network_id, data, length);
		return true;
	}

	bool NetworkManager::DoAsyncSendData(NetworkID network_id, char *data, int length)
	{
		SocketPtr socket_ptr = m_network_id_socket_map[network_id];
			boost::asio::async_write(socket_ptr->GetSocket(),
				boost::asio::buffer(data, length),
				boost::bind(&NetworkManager::OnSendData, this, socket_ptr, boost::asio::placeholders::error));
		return true;
	}

	bool NetworkManager::AsyncRun()
	{
		if (io_service_running)
			return false;

		boost::thread thread_(boost::bind(&boost::asio::io_service::run, &m_io_service));
		thread_.detach();
		io_service_running = true;
		return true;
	}

	bool NetworkManager::SyncRun()
	{
		if (io_service_running)
			return false;

		boost::thread thread_(boost::bind(&boost::asio::io_service::run, &m_io_service));
		io_service_running = true;
		thread_.join();
		return true;
	}

	void NetworkManager::OnAccept(SocketPtr socket_ptr)
	{
		if (m_socket_set.find(socket_ptr) != m_socket_set.end()) // aleady has one
		{
			return;
		}
		Port local_port = socket_ptr->GetSocket().local_endpoint().port();
		IPAddr remote_ip_addr = socket_ptr->GetSocket().remote_endpoint().address().to_v4().to_string();
		Port remote_port = socket_ptr->GetSocket().remote_endpoint().port();
		std::cout<<"Accept successful : local_port=["<<local_port<<"], remote["<< remote_ip_addr << "-" << remote_port << "]" << std::endl;

		m_socket_set.insert(socket_ptr);
		NetworkID next_id = this->GetNetworkID();
		m_network_id_socket_map[next_id] = socket_ptr;
	}

	void NetworkManager::OnConnect(SocketPtr socket_ptr)
	{
		if (m_socket_set.find(socket_ptr) != m_socket_set.end()) // aleady has one
		{
			return;
		}
		Port local_port = socket_ptr->GetSocket().local_endpoint().port();
		IPAddr remote_ip_addr = socket_ptr->GetSocket().remote_endpoint().address().to_v4().to_string();
		Port remote_port = socket_ptr->GetSocket().remote_endpoint().port();
		std::cout<<"Connect successful : local_port=["<<local_port<<"], remote["<< remote_ip_addr << "-" << remote_port << "]" << std::endl;

		m_socket_set.insert(socket_ptr);
		NetworkID next_id = this->GetNetworkID();
		m_network_id_socket_map[next_id] = socket_ptr;
	}

	NetworkID NetworkManager::GetNetworkID()
	{
		if (m_free_netid_stack.size() > 0)
		{
			int netid = m_free_netid_stack.top();
			m_free_netid_stack.pop();
			return netid;
		}
		
		m_cur_max_netid ++;
		return m_cur_max_netid;
	}

	void NetworkManager::OnDisconnect(SocketPtr socket_ptr)
	{
		std::cout<<"disconnected";
		socket_ptr->GetSocket().close();
	}

	void NetworkManager::OnSendData(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			// send successful
		}
		else
		{
			OnDisconnect(socket_ptr);
		}
	}
}