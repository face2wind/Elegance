#ifndef _NETWORK_HPP_
#define _NETWORK_HPP_

#include "network/i_network.hpp"
#include "network/socket_data.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <set>
#include <stack>
#include <vector>
#include <list>

namespace face2wind
{
using boost::asio::ip::tcp;

class AcceptSession;
class ConnectSession;

/************************************************************************
 *  Network Class
 ************************************************************************/
class Network
{
  struct MessageBuffer
  {
    MessageBuffer() : m_buffer(NULL), m_buffer_size(0) {}
    ~MessageBuffer()
    {
      this->ChangeBufferSize(0);
    }

    char *GetBuffer() { return m_buffer; }
    int GetBufferSize() { return m_buffer_size; }

    bool SetBuffer(char *buff, int length)
    {
      if (0 >= length)
        return false;

      this->ChangeBufferSize(length);
      memcpy(m_buffer, buff, length);

      return true;
    }

    bool ChangeBufferSize(int size)
    {
      m_buffer_size = size;
      if (NULL != m_buffer) delete[] m_buffer;
      if (size > 0)
      {
        m_buffer = new char[size];
        memset(m_buffer, 0, m_buffer_size);
      }

      return true;
    }
    char *m_buffer;
    int m_buffer_size;
  };

  typedef boost::shared_ptr<MessageBuffer> MessageBufferPtr;

 public:
  friend class AcceptSession;
  friend class ConnectSession;

  ~Network();
  static Network *GetInstance();

  bool RegistHandler(INetworkHandler *handler);
  bool AsyncListen(int port);
  bool AsyncConnect(const std::string &host, Port port);
  bool AsyncSendData(NetworkID network_id, const char *data, int length);
  bool Disconnect(NetworkID network_id);
	bool HasConnected(NetworkID network_id);

  bool AsyncRun();
  bool SyncRun();
  void Stop();

 protected:
  Network();

  void OnAccept(SocketPtr socket_ptr);
  void OnConnect(SocketPtr socket_ptr, bool is_success);
  void OnRecv(SocketPtr socket_ptr);
  void OnSendData(SocketPtr socket_ptr, const boost::system::error_code& error);
  void OnDisconnect(SocketPtr socket_ptr);

  NetworkID GetIdleNetworkID();

	bool DoAsyncSendData(NetworkID network_id, MessageBufferPtr buff);

 private:
  boost::asio::io_service m_io_service;
  boost::asio::signal_set m_signals;
  bool io_service_running;

  std::vector<AcceptSession*> m_accept_session_list;
  std::vector<ConnectSession*> m_connect_session_list;

  std::map<NetworkID, SocketPtr> m_network_id_socket_map;
  std::map<std::string, NetworkID> m_key_to_network_id_map;
  std::set<SocketPtr> m_socket_set;

  std::stack<NetworkID> m_free_netid_stack;
  NetworkID m_cur_max_netid;

  std::list<INetworkHandler*> m_network_handler_list;
};

}

#endif
