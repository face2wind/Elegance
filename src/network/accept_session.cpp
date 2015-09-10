#include "network/network.hpp"
#include "network/accept_session.hpp"
#include "common/debug_message.hpp"

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

    //socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
    socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
    boost::asio::async_read(socket_ptr->GetSocket(),
                            boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
                            boost::bind(&AcceptSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));

    this->AsyncListen();
  }
  else
  {
    std::stringstream ss;
    ss << "AcceptSession::OnAccept Error : " << error.message();
    DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());

		// 监听失败，调用上层接口 xxxxxxx
  }
}

void AcceptSession::OnRecvHead(SocketPtr socket_ptr, const boost::system::error_code& error)
{
  if (!error)
  {
    char *read_buff = socket_ptr->GetBuffer();
    MessageHeader body_length = *(MessageHeader*)(read_buff);
    socket_ptr->ChangeBufferSize(body_length);
    boost::asio::async_read(socket_ptr->GetSocket(),
                            boost::asio::buffer(socket_ptr->GetBuffer(), body_length),
                            boost::bind(&AcceptSession::OnRecvBody, this, socket_ptr, boost::asio::placeholders::error));
  }
  else
  {
    std::stringstream ss;
    ss << "AcceptSession::OnRecvHead Error : " << error.message();
    DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());

    if (NULL != m_network_mgr && socket_ptr->GetSocket().is_open())
      m_network_mgr->OnDisconnect(socket_ptr);
  }
}

void AcceptSession::OnRecvBody(SocketPtr socket_ptr, const boost::system::error_code& error)
{
  if (!error)
  {
    m_network_mgr->OnRecv(socket_ptr);

    if (socket_ptr->GetSocket().is_open())
    {
      socket_ptr->ChangeBufferSize(MESSAGE_HEADER_LENGTH);
      boost::asio::async_read(socket_ptr->GetSocket(),
                              boost::asio::buffer(socket_ptr->GetBuffer(), MESSAGE_HEADER_LENGTH),
                              boost::bind(&AcceptSession::OnRecvHead, this, socket_ptr, boost::asio::placeholders::error));
    }
  }
  else
  {
    if (error == boost::asio::error::eof)
    {
      std::stringstream ss;
      ss << "AcceptSession::OnRecvBody Error : " << error.message();
      DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());
    }

    if (NULL != m_network_mgr && socket_ptr->GetSocket().is_open())
      m_network_mgr->OnDisconnect(socket_ptr);
  }
}

}
