#ifndef _ACCEPT_SESSION_HPP_
#define _ACCEPT_SESSION_HPP_

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

	class NetworkManager;

	class AcceptSession
	{
		friend class NetworkManager;

	public:
		AcceptSession(boost::asio::io_service &io_service, tcp::endpoint new_endpoint);
		~AcceptSession(){}

		void AsyncListen();
		void OnAccept(SocketPtr socket_ptr, const boost::system::error_code& error);
		void OnRecvHead(SocketPtr socket_ptr, const boost::system::error_code& error);
		void OnRecvBody(SocketPtr socket_ptr, const boost::system::error_code& error);

	private:
		NetworkManager *m_network_mgr;
		boost::asio::io_service &m_io_service;
		tcp::acceptor m_acceptor;

	};

}

#endif