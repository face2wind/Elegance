#include "socket_accept.hpp"
#include "i_socket_handler.hpp"

#include <iostream>

namespace face2wind {

SocketAccept::SocketAccept() : handler_(NULL), listening_(false)
{
}

SocketAccept::~SocketAccept()
{
}

void SocketAccept::ResetHandler(ISocketHandler *handler)
{
  handler_ = handler;
}

bool SocketAccept::Listen(Port port)
{
  if (listening_)
    return false;

#ifdef __LINUX__
  struct sockaddr_in local_addr_;
  bzero(&local_addr_, sizeof(local_addr_));
  local_addr_.sin_family = AF_INET;
  local_addr_.sin_addr.s_addr = htons(INADDR_ANY);
  local_addr_.sin_port = htons(port);

  local_sock_ = socket(PF_INET,SOCK_STREAM,0);
  if(local_sock_ < 0)
    return false;
  
  int sock_reuse_on = 1;
  setsockopt(local_sock_, SOL_SOCKET, SO_REUSEADDR, &sock_reuse_on, sizeof(sock_reuse_on));

  if (-1 == bind(local_sock_, (struct sockaddr *)&local_addr_, sizeof(local_addr_)))
    return false;

  if (-1 == listen(local_sock_, MAX_BACKLOG))
    return false;

  epoll_fd_ = epoll_create(MAX_EPOLL_EVENTS);
  if (-1 == epoll_fd_)
    return false;

  struct epoll_event event;
  event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
  event.data.fd = local_sock_;

  if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, local_sock_, &event))
    return false;

  int fd_count = 0;
  socklen_t addr_in_len = (socklen_t)sizeof(struct sockaddr_in);
  struct sockaddr_in remote_addr;
  
  while(true)
  {
    std::cout<<"[server] start epoll wait ..."<<std::endl;
    fd_count = epoll_wait(epoll_fd_, epoll_event_list_, MAX_EPOLL_EVENTS, -1);
    if (-1 == fd_count)
      return false;

    for (int index = 0; index < fd_count; ++ index)
    {
      
      if (epoll_event_list_[index].data.fd == local_sock_)
      {
        int cur_sock = accept(local_sock_, (struct sockaddr *)&remote_addr, &addr_in_len);
        if (-1 == cur_sock)
          return false;

        // set nonblocking
        int opts = fcntl(cur_sock, F_GETFL);
        if (opts < 0)
          return false;
        opts = opts | O_NONBLOCK;
        if (fcntl(cur_sock, F_SETFL, opts) < 0)
          return false;

        event.events = EPOLLIN | EPOLLET | EPOLLHUP | EPOLLERR;
        event.data.fd = cur_sock;
        if (-1 == epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, cur_sock, &event))
          return false;

        Endpoint cur_endpoint(inet_ntoa(remote_addr.sin_addr), remote_addr.sin_port);
        
        sock_endpoint_map_[cur_sock] = cur_endpoint;
        endpoint_sock_map_[cur_endpoint] = cur_sock;
        
        std::cout<<"[server] accept succ from - "<<cur_endpoint.ip_addr<<":"<<cur_endpoint.port<<std::endl;
        if (nullptr != handler_)
          handler_->OnAccept(cur_endpoint.ip_addr, cur_endpoint.port);
        
        const char str[] = "123456789";
        this->Write(cur_endpoint.ip_addr, cur_endpoint.port, str, sizeof(str));
      }
      else if (epoll_event_list_[index].events & EPOLLOUT)
      {
        std::cout<<"[server] epoll out......"<<std::endl;
        const char str[] = "now i can write!\n";
        if (-1 == send(epoll_event_list_[index].data.fd, str, sizeof(str), 0))
          return false;
      }
      else if (epoll_event_list_[index].events & EPOLLIN)
      {
        std::cout<<"[server] epoll in ......"<<std::endl;
        int read_size = read(epoll_event_list_[index].data.fd, buff_, MAX_SOCKET_MSG_BUFF_LENGTH);
        if (read_size > 0)
        {
          std::cout<<"[server] receive : "<<buff_<<std::endl;
        }
      }
      else
      {
        
        close(epoll_event_list_[index].data.fd);
      }
    }
  }
#endif

  listening_ = true;  
  return true;
}

bool SocketAccept::Write(IPAddr ip, Port port, const char *data, int length)
{
  if (nullptr == data || length <= 0)
    return false;
  
  auto sock_it = endpoint_sock_map_.find(Endpoint(ip, port));
  if (sock_it == endpoint_sock_map_.end())
    return false;

  int cur_sock = sock_it->second;
  if (-1 == send(cur_sock, data, length, 0))
    return false;
  
  return true;
}

}
