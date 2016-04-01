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

  remote_ip_addr_ = ip;
  remote_port_ = port;
  
  if (nullptr != handler_)
    handler_->OnConnect(remote_ip_addr_, remote_port_);
  
  return true;
}

bool SocketConnect::Run()
{
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
  event.data.fd = local_sock_;

  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, local_sock_, &event))
    return false;

  int fd_count = 0;
  
  while(true)
  {
    std::cout<<"[connect] start epoll wait ..."<<std::endl;
    fd_count = epoll_wait(epoll_fd_, epoll_event_list_, MAX_EPOLL_EVENTS, -1);
    if (-1 == fd_count)
      return false;

    bool socket_error = false;
    for (int index = 0; index < fd_count; ++ index)
    {
      if (epoll_event_list_[index].events & EPOLLOUT)
      {
        std::cout<<"[connect] epoll out......"<<std::endl;
      }
      else if (epoll_event_list_[index].events & EPOLLIN)
      {
        std::cout<<"[connect] epoll in ......"<<std::endl;
        
        int read_size = read(epoll_event_list_[index].data.fd, buff_, MAX_SOCKET_MSG_BUFF_LENGTH);
        if (read_size > 0)
        {
          while (read_size > 0)
          {
            if (nullptr != handler_)
              handler_->OnRecv(remote_ip_addr_, remote_port_, buff_, read_size);
            
            read_size = read(epoll_event_list_[index].data.fd, buff_, MAX_SOCKET_MSG_BUFF_LENGTH);
          }
                 
          const char str[] = "i love you 222 !\n";
          //if (-1 == send(epoll_event_list_[index].data.fd, str, sizeof(str), 0))
          if (-1 == send(local_sock_, str, sizeof(str), 0))
            return false;
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
      }
    }
    
    if (socket_error)
      break;
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
