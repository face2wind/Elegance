#include "network/network_manager.hpp"
#include "network/accept_session.hpp"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <sstream>

namespace face2wind
{

	AcceptSession::AcceptSession(boost::asio::io_service& io_service, tcp::endpoint new_endpoint)
		:m_network_mgr(NULL), m_io_service(io_service), m_acceptor(m_io_service, new_endpoint)
	{

	}

	void AcceptSession::AsyncListen()
	{
		SocketPtr socket_ptr(new SocketData(m_io_service));
		m_acceptor.async_accept(socket_ptr->GetSocket(), 
			boost::bind(&AcceptSession::OnAccept, this, socket_ptr, boost::asio::placeholders::error));
	}

	void AcceptSession::OnAccept(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			m_network_mgr->OnAccept(socket_ptr);

			socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
			boost::asio::async_read(socket_ptr->GetSocket(),
				boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
				boost::bind(&AcceptSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));

			this->AsyncListen();
		}
		else
		{
			std::cout<<"accept error : "<<error.message()<<std::endl;
		}
	}

	void AcceptSession::OnRecvHead(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			MessageHeader body_length = *(MessageHeader*)(socket_ptr->GetBuffer());
			socket_ptr->ChangeBufferSize(body_length);
			boost::asio::async_read(socket_ptr->GetSocket(),
				boost::asio::buffer(socket_ptr->GetBuffer(), body_length),
				boost::bind(&AcceptSession::OnRecvBody, this, socket_ptr, boost::asio::placeholders::error));
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

	void AcceptSession::OnRecvBody(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
			boost::asio::async_read(socket_ptr->GetSocket(),
				boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
				boost::bind(&AcceptSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));
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

	void AcceptSession::OnDisconnect(NetworkID network_id)
	{

	}

}