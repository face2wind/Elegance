#include "network/network.hpp"
#include "network/connect_session.hpp"
#include "common/debug_message.hpp"

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
			socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
			boost::asio::async_read(socket_ptr->GetSocket(),
				boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
				boost::bind(&ConnectSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));

			if (NULL != m_network_mgr && socket_ptr->GetSocket().is_open())
				m_network_mgr->OnConnect(socket_ptr, true);
		}
		else
		{
			std::stringstream ss;
			ss << "ConnectSession::OnConnect Error : " << error.message();
			DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());

			if (NULL != m_network_mgr)
				m_network_mgr->OnConnect(socket_ptr, false);
			// 连接失败，调用上层接口 xxxxxxx
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
			std::stringstream ss;
			ss << "ConnectSession::OnRecvHead Error: " << error.message();
			DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());

			if (NULL != m_network_mgr && socket_ptr->GetSocket().is_open())
				m_network_mgr->OnDisconnect(socket_ptr);
		}
	}

	void ConnectSession::OnRecvBody(SocketPtr socket_ptr, const boost::system::error_code& error)
	{
		if (!error)
		{
			if (NULL != m_network_mgr)
				m_network_mgr->OnRecv(socket_ptr);

			if (socket_ptr->GetSocket().is_open())
			{
				socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
				boost::asio::async_read(socket_ptr->GetSocket(),
					boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
					boost::bind(&ConnectSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));
			}
		}
		else
		{
			std::stringstream ss;
			ss << "ConnectSession::OnRecvBody Error : " << error.message();
			DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());

			if (NULL != m_network_mgr && socket_ptr->GetSocket().is_open())
				m_network_mgr->OnDisconnect(socket_ptr);
		}
	}

}