#include "network/network_manager.hpp"
#include "network/connect_session.hpp"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <sstream>

namespace face2wind
{
	ConnectSession::ConnectSession(boost::asio::io_service& io_service, tcp::resolver::iterator iterator)
		:m_network_mgr(NULL), m_io_service(io_service), m_iterator(iterator)
	{

	}

	void ConnectSession::AsyncConnect()
	{
		SocketPtr socket_ptr(new SocketData(m_io_service));
		socket_ptr->GetSocket().async_connect
			(*m_iterator, boost::bind(&ConnectSession::OnConnect, this, socket_ptr, boost::asio::placeholders::error));
	}

	void ConnectSession::OnConnect(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			m_network_mgr->OnConnect(socket_ptr, true);

			socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
			boost::asio::async_read(socket_ptr->GetSocket(),
				boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
				boost::bind(&ConnectSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));
		}
		else
		{
			std::cout<<"connect error : "<<error.message()<<std::endl;
			m_network_mgr->OnConnect(socket_ptr, false);
		}
	}

	void ConnectSession::OnRecvHead(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			MessageHeader body_length = *(MessageHeader*)(socket_ptr->GetBuffer());
			socket_ptr->ChangeBufferSize(body_length);
			boost::asio::async_read(socket_ptr->GetSocket(),
				boost::asio::buffer(socket_ptr->GetBuffer(), body_length),
				boost::bind(&ConnectSession::OnRecvBody, this, socket_ptr, boost::asio::placeholders::error));
		}
		else
		{
			if (error == boost::asio::error::eof)
				std::cout<<"been disconnected : "<<error.message()<<std::endl;
			else
				std::cout<<"read head error : "<<error.message()<<std::endl;

			if (NULL != m_network_mgr)
				m_network_mgr->OnDisconnect(socket_ptr);
		}
	}

	void ConnectSession::OnRecvBody(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			m_network_mgr->OnRecv(socket_ptr);
			socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
			boost::asio::async_read(socket_ptr->GetSocket(),
				boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
				boost::bind(&ConnectSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));
		}
		else
		{
			if (error == boost::asio::error::eof)
				std::cout<<"been disconnected : "<<error.message()<<std::endl;
			else
				std::cout<<"read body error : "<<error.message()<<std::endl;

			if (NULL != m_network_mgr)
				m_network_mgr->OnDisconnect(socket_ptr);
		}
	}

}