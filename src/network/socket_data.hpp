#ifndef _SOCKET_DATA_HPP_
#define _SOCKET_DATA_HPP_

#include <string>
#include <boost/asio.hpp>
#include "common/debug_message.hpp"

namespace face2wind
{
using boost::asio::ip::tcp;

class SocketData
{
 public:
  SocketData(boost::asio::io_service& io_service) 
      : m_socket(io_service), m_buffer(NULL), m_buffer_size(0),
        m_local_port(0), m_remote_port(0), m_remote_ip_addr("0.0.0.0"), m_key_str("_invalid_key_") {}
  ~SocketData() { if (NULL != m_buffer) delete [] m_buffer; }

  tcp::socket &GetSocket() { return m_socket; }
  char *GetBuffer() { return m_buffer; }
  int GetBufferSize() { return m_buffer_size; }
  Port GetLocalPort() { return m_local_port; }
  Port GetRemotePort() { return m_remote_port; }
  IPAddr GetRemoteIP() { return m_remote_ip_addr; }
  std::string GetUniqueKey() { return m_key_str; }

  bool ChangeBufferSize(int size)
  {
    if (0 >= size)
      return false;

    if (NULL != m_buffer)
      delete [] m_buffer;
		m_buffer = new char[size];
		m_buffer_size = size;
    memset(m_buffer, 0, m_buffer_size);

    return true;
  }

  bool InitSocketData()
  {
    if (!m_socket.is_open())
      return false;

    boost::system::error_code ec;
    boost::asio::ip::tcp::endpoint local_endpoint = m_socket.local_endpoint(ec);
    m_local_port = 0;
    if (!ec)
    {
      m_local_port = local_endpoint.port();
    }
    else
    {
      std::stringstream ss;
      ss << "NetworkManager::OnAccept Get remote_endpoint error : " << ec.message();
      DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());
      return false;
    }
    m_remote_ip_addr = "0.0.0.0";
    m_remote_port = 0;
    boost::asio::ip::tcp::endpoint remote_endpoint = m_socket.remote_endpoint(ec);
    if (!ec)
    {
      m_remote_ip_addr = remote_endpoint.address().to_v4().to_string();
      m_remote_port = remote_endpoint.port();
    }
    else
    {
      std::stringstream ss;
      ss << "NetworkManager::OnAccept Get remote_endpoint error : " << ec.message();
      DebugMessage::GetInstance()->ShowMessage(DebugMessageType::DEBUG_MESSAGE_TYPE_BASE_NETWORK, ss.str());
      return false;
    }

    std::stringstream keyStream;
    keyStream << m_local_port << "_" << m_remote_ip_addr << "_" << m_remote_port;
    m_key_str = keyStream.str();

    return true;
  }


 protected:
  tcp::socket m_socket;
  char *m_buffer;
  int m_buffer_size;
  Port m_local_port;
  Port m_remote_port;
  IPAddr m_remote_ip_addr;
  std::string m_key_str;
};

typedef boost::shared_ptr<SocketData> SocketPtr;

typedef unsigned int MessageHeader;
static const int MESSAGE_HEADER_LENGTH = sizeof(MessageHeader);
}

#endif
