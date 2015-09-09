#include "network/network.hpp"
#include "network/connect_session.hpp"
#include "network/accept_session.hpp"
#include "common/debug_message.hpp"

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <sstream>

namespace face2wind
{

Network::Network() : m_signals(m_io_service), io_service_running(false), m_cur_max_netid(0)
{
  m_signals.add(SIGINT);
  m_signals.add(SIGILL);
  m_signals.add(SIGFPE);
  m_signals.add(SIGSEGV);
  m_signals.add(SIGTERM);
  m_signals.add(SIGABRT);
#if defined(SIGBREAK)
  m_signals.add(SIGBREAK);
#endif // defined(SIGQUIT)
#if defined(SIGQUIT)
  m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)

  m_signals.async_wait(boost::bind(&Network::Stop, this));
}

Network::~Network()
{
  for (std::vector<AcceptSession*>::iterator it = m_accept_session_list.begin(); it != m_accept_session_list.end(); ++ it)
    delete *it;
  m_accept_session_list.clear();

  for (std::vector<ConnectSession*>::iterator it = m_connect_session_list.begin(); it != m_connect_session_list.end(); ++ it)
    delete *it;
  m_connect_session_list.clear();

  this->Stop();
}

Network *Network::GetInstance()
{
  static Network instance;
  return &instance;
}

bool Network::RegistHandler(INetworkHandler *handler)
{
  if (NULL == handler)
    return false;

  m_network_handler_list.push_back(handler);
  return true;
}

bool Network::AsyncListen(int port)
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

bool Network::AsyncConnect(const std::string &host, Port port)
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

bool Network::AsyncSendData(NetworkID network_id, const char *data, int length)
{
  if (false == io_service_running)
    return false;

  if (0 >= length)
    return false;
  if (m_network_id_socket_map.count(network_id) <= 0)
    return false;

  MessageBufferPtr buff(new MessageBuffer());

  // insert the head on data begin
  buff->ChangeBufferSize(length + MESSAGE_HEADER_LENGTH);
  char *real_buff = buff->GetBuffer();
  MessageHeader *head = (MessageHeader*)real_buff;
  *head = length;
  char *body = real_buff + MESSAGE_HEADER_LENGTH;
  memcpy(body, data, length);
		
  if (io_service_running)
    m_io_service.post(boost::bind(&Network::DoAsyncSendData, this, network_id, buff));
  else
    this->DoAsyncSendData(network_id, buff);
  return true;
}

bool Network::Disconnect(NetworkID network_id)
{
  if (m_network_id_socket_map.count(network_id) <= 0)
    return false;

  this->OnDisconnect(m_network_id_socket_map[network_id]);
  return true;
}

bool Network::HasConnected(NetworkID network_id)
{
	if (m_network_id_socket_map.count(network_id) <= 0)
		return false;
	else
		return true;
}

bool Network::AsyncRun()
{
  if (io_service_running)
    return false;

  boost::thread thread_(boost::bind(&boost::asio::io_service::run, &m_io_service));
  thread_.detach();
  io_service_running = true;
  return true;
}

bool Network::SyncRun()
{
  if (io_service_running)
    return false;

  boost::thread thread_(boost::bind(&boost::asio::io_service::run, &m_io_service));
  io_service_running = true;
  thread_.join();
  return true;
}

void Network::OnAccept(SocketPtr socket_ptr)
{
  if (m_socket_set.find(socket_ptr) != m_socket_set.end()) // aleady has one
  {
    return;
  }

  if (!socket_ptr->InitSocketData())
  {
    this->OnDisconnect(socket_ptr);
    return;
  }

  std::stringstream ss;
  ss << "Accept successful : local_port=[" << socket_ptr->GetLocalPort() << "], remote[" << socket_ptr->GetRemoteIP() << "-" << socket_ptr->GetRemotePort() << "]";
  DebugMessage::GetInstance()->ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());

  m_socket_set.insert(socket_ptr);
  NetworkID network_id = this->GetIdleNetworkID();
  m_network_id_socket_map[network_id] = socket_ptr;
  m_key_to_network_id_map[socket_ptr->GetUniqueKey()] = network_id;

  for(std::list<INetworkHandler*>::iterator it = m_network_handler_list.begin(); it != m_network_handler_list.end(); ++ it)
  {
    (*it)->OnAccept(network_id, socket_ptr->GetLocalPort(), socket_ptr->GetRemoteIP(), socket_ptr->GetRemotePort());
  }
}

void Network::OnConnect(SocketPtr socket_ptr, bool is_success)
{
  if (m_socket_set.find(socket_ptr) != m_socket_set.end()) // aleady has one
  {
    return;
  }

  NetworkID network_id = 0;
  if (is_success)
	{
		if (!socket_ptr->InitSocketData())
		{
			this->OnDisconnect(socket_ptr);
			return;
		}

    std::stringstream ss;
    ss <<"Connect successful : local_port=["<< socket_ptr->GetLocalPort()<<"], remote["<< socket_ptr->GetRemoteIP() << "-" << socket_ptr->GetRemotePort() << "]";
    DebugMessage::GetInstance()->ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());

    m_socket_set.insert(socket_ptr);
    network_id = this->GetIdleNetworkID();
    m_network_id_socket_map[network_id] = socket_ptr;
    m_key_to_network_id_map[socket_ptr->GetUniqueKey()] = network_id;
  }

  for(std::list<INetworkHandler*>::iterator it = m_network_handler_list.begin(); it != m_network_handler_list.end(); ++ it)
  {
    (*it)->OnConnect(is_success, network_id, socket_ptr->GetLocalPort(), socket_ptr->GetRemoteIP(), socket_ptr->GetRemotePort());
  }
}

void Network::OnRecv(SocketPtr socket_ptr)
{
  char *buff = socket_ptr->GetBuffer();
  int buff_size = socket_ptr->GetBufferSize();
  if (NULL == buff || 0 >= buff_size)
    return;

  std::string key = socket_ptr->GetUniqueKey();
  if (0 >= m_key_to_network_id_map.count(key))
    return;

  int network_id = m_key_to_network_id_map[key];

	for (std::list<INetworkHandler*>::iterator it = m_network_handler_list.begin(); it != m_network_handler_list.end(); ++it)
	{
		(*it)->OnRecv(network_id, buff, buff_size);
	}
}

void Network::OnSendData(SocketPtr socket_ptr, const boost::system::error_code& error)
{
  if (!error)
  {
    //std::cout<<"NetworkManager::OnSendData Success!"<<std::endl;
  }
  else
  {
    std::stringstream ss;
    ss << "NetworkManager::OnSendData Error" << error.message();
    DebugMessage::GetInstance()->ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
    this->OnDisconnect(socket_ptr);
  }
}

void Network::OnDisconnect(SocketPtr socket_ptr)
{
	std::stringstream ss;
	ss << "disconnected : local_port=[" << socket_ptr->GetLocalPort() << "], remote[" << socket_ptr->GetRemoteIP() << "-" << socket_ptr->GetRemotePort() << "]";
	DebugMessage::GetInstance()->ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());

	std::map<std::string, NetworkID>::iterator it = m_key_to_network_id_map.find(socket_ptr->GetUniqueKey());
  if (it != m_key_to_network_id_map.end())
  {
    m_socket_set.erase(socket_ptr);
    NetworkID netid = it->second;
    m_network_id_socket_map.erase(netid);
    m_free_netid_stack.push(netid);
    m_key_to_network_id_map.erase(it);

    for(std::list<INetworkHandler*>::iterator it = m_network_handler_list.begin(); it != m_network_handler_list.end(); ++ it)
    {
      (*it)->OnDisconnect(netid);
    }
  }
  socket_ptr->GetSocket().close();
}

// The server is stopped by cancelling all outstanding asynchronous
// operations. Once all operations have finished the io_service::run()
// call will exit.
void Network::Stop()
{
  if (io_service_running)
	{
		m_io_service.post(boost::bind(&boost::asio::io_service::stop, &m_io_service));
		io_service_running = false;
  }
}

NetworkID Network::GetIdleNetworkID()
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

bool Network::DoAsyncSendData(NetworkID network_id, MessageBufferPtr buff)
{
	if (false == io_service_running)
		return false;
	if (m_network_id_socket_map.count(network_id) <= 0)
		return false;

	SocketPtr socket_ptr = m_network_id_socket_map[network_id];
	if (socket_ptr->GetSocket().is_open())
	{
		boost::asio::async_write(socket_ptr->GetSocket(),
			boost::asio::buffer(buff->GetBuffer(), buff->GetBufferSize()),
			boost::bind(&Network::OnSendData, this, socket_ptr, boost::asio::placeholders::error));
	}
	return true;
}


}
