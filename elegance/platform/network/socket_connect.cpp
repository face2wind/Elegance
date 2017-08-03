#include "socket_connect.hpp"
#include "common/debug_message.hpp"

namespace face2wind {

SocketConnect::SocketConnect() : handler_(nullptr), running_(false), remote_port_(0), local_port_(0), local_sock_(0)
{
}

SocketConnect::~SocketConnect()
{
}

void SocketConnect::ResetHandler(ISocketHandler *handler)
{
  handler_ = handler;
}

bool SocketConnect::Disconnect()
{
  running_ = false;
  return true;
}

#ifdef __LINUX__
bool SocketConnect::Connect(IPAddr ip, Port port)
{
  if (running_)
    return false;

  struct sockaddr_in local_addr_;
  bzero(&local_addr_, sizeof(local_addr_));
  local_addr_.sin_family = AF_INET;
  local_addr_.sin_addr.s_addr = inet_addr(ip.c_str());
  local_addr_.sin_port = htons(port);

  local_sock_ = socket(PF_INET,SOCK_STREAM,0);
  if(local_sock_ < 0)
    return false;
  
  int sock_reuse_on = 1;
  setsockopt(local_sock_, SOL_SOCKET, SO_REUSEADDR, &sock_reuse_on, sizeof(sock_reuse_on));

  if (-1 == connect(local_sock_, (struct sockaddr*)&(local_addr_), sizeof(local_addr_)))
    return false;

  struct sockaddr_in connected_addr;
  socklen_t connected_addr_len = sizeof(connected_addr);
  getsockname(local_sock_, (struct sockaddr*)&connected_addr, &connected_addr_len);
  local_port_ = ntohs(connected_addr.sin_port);

  // set nonblocking
  int opts = fcntl(local_sock_, F_GETFL);
  if (opts < 0)
    return false;
  opts = opts | O_NONBLOCK;
  if (fcntl(local_sock_, F_SETFL, opts) < 0)
    return false;
  
  epoll_fd_ = epoll_create(MAX_EPOLL_EVENTS);
  if (-1 == epoll_fd_)
    return false;

  remote_ip_addr_ = ip;
  remote_port_ = port;
  
  if (nullptr != handler_)
    handler_->OnConnect(remote_ip_addr_, remote_port_, local_port_);
  
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
  event.data.fd = local_sock_;

  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, local_sock_, &event))
    return false;

  int fd_count = 0;
  
  running_ = true;
  while(running_)
  {
    fDebugWithHead(DebugMessageType::BASE_NETWORK) << "SocketConnect::Connect start epoll wait" << fDebugEndl;
    
    fd_count = epoll_wait(epoll_fd_, epoll_event_list_, MAX_EPOLL_EVENTS, -1);
    if (-1 == fd_count)
      return false;

    bool socket_error = false;
    for (int index = 0; index < fd_count; ++ index)
    {
      if (epoll_event_list_[index].events & EPOLLOUT)
      {
        //std::cout<<"[connect] epoll out......"<<std::endl;
      }
      else if (epoll_event_list_[index].events & EPOLLIN)
      {
        //std::cout<<"[connect] epoll in ......"<<std::endl;
        
        int read_size = read(epoll_event_list_[index].data.fd, buff_, MAX_SOCKET_MSG_BUFF_LENGTH);
        if (read_size > 0)
        {
          while (read_size > 0)
          {
            buff_[read_size] = '\0';
            if (nullptr != handler_)
              handler_->OnRecv(remote_ip_addr_, remote_port_, local_port_, buff_, read_size);
            
            read_size = read(epoll_event_list_[index].data.fd, buff_, MAX_SOCKET_MSG_BUFF_LENGTH);
          }
        }
        else
        {
          // remove listen in epoll
          struct epoll_event ev;
          ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
          ev.data.fd = epoll_event_list_[index].data.fd;
          epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, epoll_event_list_[index].data.fd, &ev);

          close(epoll_event_list_[index].data.fd);
          socket_error = true;
          
          if (nullptr != handler_)
            handler_->OnDisconnect(remote_ip_addr_, remote_port_, local_port_);
        }
      }
      else
      {
        // remove listen in epoll
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
        ev.data.fd = epoll_event_list_[index].data.fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, epoll_event_list_[index].data.fd, &ev);
          
        close(epoll_event_list_[index].data.fd);
        socket_error = true;
        
        if (nullptr != handler_)
          handler_->OnDisconnect(remote_ip_addr_, remote_port_, local_port_);
      }
    }
    
    if (socket_error)
      break;
  }

  running_ = false;
  return true;
}

bool SocketConnect::Write(const char *data, int length)
{
  //if (-1 == send(epoll_event_list_[index].data.fd, str, sizeof(str), 0))
  if (-1 == send(local_sock_, data, length, 0))
    return false;
  
  return true;
}
#endif

#ifdef __WINDOWS__

bool SocketConnect::Connect(IPAddr ip, Port port)
{
  if (running_)
    return false;

  WSADATA          wsaData;
  SOCKADDR_IN      ServerAddr;
  int Ret;
  if ((Ret = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0)
  {
    fDebugWithHead(DebugMessageType::BASE_NETWORK) << "SocketConnect::Connect Error:WSAStartup failed with " << Ret << fDebugEndl;
    return false;
  }

  if ((local_sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
  {
    fDebugWithHead(DebugMessageType::BASE_NETWORK) << "SocketConnect::Connect Error:socket failed with " << WSAGetLastError() << fDebugEndl;
    WSACleanup();
    return false;
  }
	
  ServerAddr.sin_family = AF_INET;
  ServerAddr.sin_port = htons(port);
  //ServerAddr.sin_addr.s_addr = inet_addr(ip.c_str());
  inet_pton(AF_INET, ip.c_str(), &ServerAddr.sin_addr);

  if (SOCKET_ERROR == connect(local_sock_, (SOCKADDR *)&ServerAddr, sizeof(ServerAddr)))
  {
    fDebugWithHead(DebugMessageType::BASE_NETWORK) << "SocketConnect::Connect Error:connect failed with " << WSAGetLastError() << fDebugEndl;
    closesocket(local_sock_);
    WSACleanup();
    return false;
  }

  struct sockaddr_in connected_addr;
  socklen_t connected_addr_len = sizeof(connected_addr);
  getsockname(local_sock_, (struct sockaddr*)&connected_addr, &connected_addr_len);
  local_port_ = ntohs(connected_addr.sin_port);

  remote_ip_addr_ = ip;
  remote_port_ = port;
  running_ = true;

  if (nullptr != handler_)
    handler_->OnConnect(remote_ip_addr_, remote_port_, local_port_);

  while (running_)
  {
    int recv_size = recv(local_sock_, buff_, MAX_SOCKET_MSG_BUFF_LENGTH, 0);
    if (0 == recv_size)
    {
      break;
    }

    if (nullptr != handler_)
      handler_->OnRecv(remote_ip_addr_, remote_port_, local_port_, buff_, recv_size);
  }

  if (nullptr != handler_)
    handler_->OnDisconnect(remote_ip_addr_, remote_port_, local_port_);

  closesocket(local_sock_);
  WSACleanup();

  running_ = false;
  return true;
}

bool SocketConnect::Write(const char *data, int length)
{
  if (!running_)
    return false;

  if (SOCKET_ERROR == send(local_sock_, data, length, 0))
  {
    fDebugWithHead(DebugMessageType::BASE_NETWORK) << "SocketConnect::Write Error:send failed with " << WSAGetLastError() << fDebugEndl;
    
    running_ = false;
    return false;
  }

  return true;
}
#endif

bool SocketConnect::CheckOnHandle(IPAddr ip, Port port)
{
  return (ip == remote_ip_addr_ && port == remote_port_);
}

}
