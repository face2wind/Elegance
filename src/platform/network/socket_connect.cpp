#include "socket_connect.hpp"
#include "i_socket_handler.hpp"

#include <iostream>

namespace face2wind {

SocketConnect::SocketConnect() : handler_(NULL), running_(false)
{
}

SocketConnect::~SocketConnect()
{
}

void SocketConnect::ResetHandler(ISocketHandler *handler)
{
  handler_ = handler;
}

bool SocketConnect::Connect(IPAddr ip, Port port)
{
  if (running_)
    return false;

#ifdef __LINUX__
  std::cout<<"linux ....."<<std::endl;
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

  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = local_sock_;

  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, local_sock_, &event))
    return false;

  int fd_count = 0;
  
  while(true)
  {
    std::cout<<"start epoll wait ..."<<std::endl;
    fd_count = epoll_wait(epoll_fd_, epoll_event_list_, MAX_EPOLL_EVENTS, -1);
    if (-1 == fd_count)
      return false;

    for (int index = 0; index < fd_count; ++ index)
    {
      if (epoll_event_list_[index].events & EPOLLOUT)
      {
        std::cout<<"here i am epoll out......"<<std::endl;
      }

      if (epoll_event_list_[index].events & EPOLLIN)
      {
        std::cout<<"here i am epoll in ......"<<std::endl;
      }
    }
  }
#endif

  running_ = true;  
  return true;
}

bool SocketConnect::Write(char *data, int length)
{
  return true;
}

}
