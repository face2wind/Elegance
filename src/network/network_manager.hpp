#ifndef _NETWORK_MANAGER_HPP_
#define _NETWORK_MANAGER_HPP_

#include "network/network_basic.hpp"
#include "network/socket_data.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <set>
#include <stack>
#include <vector>

namespace face2wind
{
	using boost::asio::ip::tcp;

	class AcceptSession;
	class ConnectSession;

	class NetworkManager
	{
	public:
		friend class AcceptSession;
		friend class ConnectSession;

		~NetworkManager();
		static NetworkManager *GetInstance();

		bool RegistHandler(INetworkHandler *handler);

		bool AsyncListen(int port);

		bool AsyncConnect(const std::string &host, Port port);

		bool AsyncSendData(NetworkID network_id, char *data, int length);
		bool DoAsyncSendData(NetworkID network_id, char *data, int length);

		bool AsyncRun();
		bool SyncRun();

	protected:
		NetworkManager();

		void OnAccept(SocketPtr socket_ptr);
		void OnConnect(SocketPtr socket_ptr);
		void OnRecv(SocketPtr socket_ptr);
		void OnDisconnect(SocketPtr socket_ptr);
		void OnSendData(SocketPtr socket_ptr, const boost::system::error_code& error);

		NetworkID GetNetworkID();

	private:
		boost::asio::io_service m_io_service;
		bool io_service_running;

		std::vector<AcceptSession*> m_accept_session_list;
		std::vector<ConnectSession*> m_connect_session_list;

		std::map<NetworkID, SocketPtr> m_network_id_socket_map;
		std::set<SocketPtr> m_socket_set;

		std::stack<NetworkID> m_free_netid_stack;
		NetworkID m_cur_max_netid;
	};

}

#endif