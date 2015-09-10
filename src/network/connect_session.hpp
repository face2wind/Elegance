#ifndef _CONNECT_SESSION_HPP_
#define _CONNECT_SESSION_HPP_

#include "network/i_network.hpp"
#include "network/socket_data.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <set>
#include <stack>
#include <vector>

namespace face2wind
{
	class Network;

	class ConnectSession
	{
		friend class Network;
	public:
		ConnectSession(boost::asio::io_service &io_service, tcp::resolver::iterator iterator);
		~ConnectSession(){}

		void AsyncConnect();

	protected:
		void OnConnect(SocketPtr socket_ptr, const boost::system::error_code& error);
		void OnRecvHead(SocketPtr socket_ptr, const boost::system::error_code& error);
		void OnRecvBody(SocketPtr socket_ptr, const boost::system::error_code& error);

	private:
		Network *m_network_mgr;
		boost::asio::io_service &m_io_service;
		tcp::resolver::iterator m_iterator;

	};
}

#endif