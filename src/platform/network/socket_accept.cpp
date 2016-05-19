#include "socket_def.hpp"
#include "socket_accept.hpp"

#include "common/debug_message.hpp"

#include <sstream>

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

#ifdef __LINUX__

bool SocketAccept::Listen(Port port)
{
  if (listening_)
    return false;

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

  local_port_ = port;
  listening_ = true;
  
  while(true)
  {
	  std::stringstream ss;
	  ss << "SocketAccept::Listen start epoll wait";
	  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());

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

        Endpoint cur_endpoint(inet_ntoa(remote_addr.sin_addr), remote_addr.sin_port, local_port_);
        
        sock_endpoint_map_[cur_sock] = cur_endpoint;
        endpoint_sock_map_[cur_endpoint] = cur_sock;

		std::stringstream ss;
		ss << "SocketAccept::Listen accept succ from - " << cur_endpoint.remote_ip_addr << ":" << cur_endpoint.remote_port;
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());

        if (nullptr != handler_)
          handler_->OnAccept(cur_endpoint.remote_ip_addr, cur_endpoint.remote_port, cur_endpoint.local_port);
      }
      else if (epoll_event_list_[index].events & EPOLLOUT)
      {
        //std::cout<<"[server] epoll out......"<<std::endl;
      }
      else if (epoll_event_list_[index].events & EPOLLIN)
      {
        //std::cout<<"[server] epoll in ......"<<std::endl;
        auto endpoint_it = sock_endpoint_map_.find(epoll_event_list_[index].data.fd);
        int read_size = read(epoll_event_list_[index].data.fd, buff_, MAX_SOCKET_MSG_BUFF_LENGTH);
        if (read_size > 0)
        {
          while (read_size > 0)
          {
            buff_[read_size] = '\0';
            if (nullptr != handler_ && endpoint_it != sock_endpoint_map_.end())
              handler_->OnRecv(endpoint_it->second.remote_ip_addr, endpoint_it->second.remote_port, endpoint_it->second.local_port, buff_, read_size);
            
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
          
		  if (nullptr != handler_ && endpoint_it != sock_endpoint_map_.end())
			  handler_->OnDisconnect(endpoint_it->second.remote_ip_addr, endpoint_it->second.remote_port, endpoint_it->second.local_port);
          if (endpoint_it != sock_endpoint_map_.end())
          {
            endpoint_sock_map_.erase(endpoint_it->second);
            sock_endpoint_map_.erase(endpoint_it);
          }
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
        
        auto endpoint_it = sock_endpoint_map_.find(epoll_event_list_[index].data.fd);
		if (nullptr != handler_ && endpoint_it != sock_endpoint_map_.end())
			handler_->OnDisconnect(endpoint_it->second.remote_ip_addr, endpoint_it->second.remote_port, endpoint_it->second.local_port);
        if (endpoint_it != sock_endpoint_map_.end())
        {
          endpoint_sock_map_.erase(endpoint_it->second);
          sock_endpoint_map_.erase(endpoint_it);
        }
      }
    }
  }

  local_port_ = 0;
  listening_ = false;  
  return true;
}

bool SocketAccept::Write(IPAddr ip, Port port, const char *data, int length)
{
  if (nullptr == data || length <= 0)
    return false;
  
  auto sock_it = endpoint_sock_map_.find(Endpoint(ip, port, local_port_));
  if (sock_it == endpoint_sock_map_.end())
    return false;

  int cur_sock = sock_it->second;
  if (-1 == send(cur_sock, data, length, 0))
    return false;
  
  return true;
}

bool SocketAccept::Disconnect(IPAddr ip, Port port)
{
  auto sock_it = endpoint_sock_map_.find(Endpoint(ip, port, local_port_));
  if (sock_it == endpoint_sock_map_.end())
    return false;

  int cur_sock = sock_it->second;
  close(cur_sock);

  return true;
}
#endif

#ifdef __WINDOWS__

DWORD WINAPI SocketAccept::ServerWorkThread(LPVOID CompletionPortID)
{
  HANDLE complationPort = (HANDLE)CompletionPortID;
  DWORD bytesTransferred;
  LPPER_HANDLE_DATA pHandleData = NULL;
  LPPER_IO_OPERATION_DATA pIoData = NULL;
  DWORD sendBytes = 0;
  DWORD recvBytes = 0;
  DWORD flags;

  while (true)
  {
    bool get_queue_error = false;
    if (GetQueuedCompletionStatus(complationPort, &bytesTransferred, (PULONG_PTR)&pHandleData, (LPOVERLAPPED *)&pIoData, INFINITE) == 0)
    {
		get_queue_error = true;

		std::stringstream ss;
		ss << "SocketAccept::ServerWorkThread GetQueuedCompletionStatus failed. Error:" << GetLastError();
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
    }
	
    if (nullptr == pIoData || nullptr == pHandleData)
      continue;

    if (bytesTransferred == 0 || get_queue_error)
    {
      // socket aleady disconnect
	  
      if (nullptr != pIoData->accept_ptr)
      {
        auto endpoint_it = pIoData->accept_ptr->sock_endpoint_map_.find(pHandleData->socket);

        if (nullptr != pIoData->accept_ptr->handler_ && endpoint_it != pIoData->accept_ptr->sock_endpoint_map_.end())
          pIoData->accept_ptr->handler_->OnDisconnect(endpoint_it->second.remote_ip_addr, endpoint_it->second.remote_port, endpoint_it->second.local_port);

        pIoData->accept_ptr->endpoint_sock_map_.erase(endpoint_it->second);
        pIoData->accept_ptr->sock_endpoint_map_.erase(endpoint_it);
        pIoData->accept_ptr->send_queue_map_.erase(pHandleData->socket);
      }

      if (bytesTransferred != 0 || !get_queue_error) // socket has not been close
      {
        if (CloseHandle((HANDLE)pHandleData->socket) == SOCKET_ERROR)
		{
			std::stringstream ss;
			ss << "SocketAccept::ServerWorkThread Close socket failed. Error:" << GetLastError();
			DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
          return 0;
        }
      }

      GlobalFree(pHandleData);
      GlobalFree(pIoData);
      continue;
    }

    pIoData->bytesTransferred += bytesTransferred;
		
    bool all_bytes_transferred = bytesTransferred >= pIoData->databuff.len;

    if (pIoData->type == IOCPHandleType::SEND)
    {
      if (all_bytes_transferred)
      {
        if (nullptr != pIoData->accept_ptr)
        {
          LPPER_IO_OPERATION_DATA new_pio_data_ptr = nullptr; // get next queue send data
          while (nullptr == new_pio_data_ptr && !pIoData->accept_ptr->send_queue_map_[pHandleData->socket].empty())
          {
            new_pio_data_ptr = pIoData->accept_ptr->send_queue_map_[pHandleData->socket].front();
            pIoData->accept_ptr->send_queue_map_[pHandleData->socket].pop();
          }

          if (nullptr != new_pio_data_ptr)
          {
            if (WSASend(pHandleData->socket, &(new_pio_data_ptr->databuff), 1, &sendBytes, 0, &(new_pio_data_ptr->overlapped), NULL) == SOCKET_ERROR)
            {
              if (WSAGetLastError() != ERROR_IO_PENDING)
			  {
				  std::stringstream ss;
				  ss << "SocketAccept::ServerWorkThread WSASend() failed. Error:" << GetLastError();
				  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
                return 0;
              }
              else
              {
                pIoData->accept_ptr->send_queue_map_[pHandleData->socket].push(NULL);
              }
            }
          }
        }
        //GlobalFree(pHandleData);
        GlobalFree(pIoData);
      }
      else
      {
        // 继续发送剩余的
        pIoData->databuff.buf += bytesTransferred;
        pIoData->databuff.len -= bytesTransferred;
        ZeroMemory(&(pIoData->overlapped), sizeof(pIoData->overlapped));

        if (WSASend(pHandleData->socket, &(pIoData->databuff), 1, &sendBytes, 0, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
        {
          if (WSAGetLastError() != ERROR_IO_PENDING)
		  {
			  std::stringstream ss;
			  ss << "SocketAccept::ServerWorkThread WSASend() failed. Error:" << GetLastError();
			  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
            return 0;
          }
        }
      }
    }
    else if (pIoData->type == IOCPHandleType::RECV)
    {
      // 接收数据，不管有没有接收完全，都直接触发事件，并重新接收下一段数据

      if (nullptr != pIoData->accept_ptr)
      {
        auto endpoint_it = pIoData->accept_ptr->sock_endpoint_map_.find(pHandleData->socket);

        if (nullptr != pIoData->accept_ptr->handler_ && endpoint_it != pIoData->accept_ptr->sock_endpoint_map_.end())
          pIoData->accept_ptr->handler_->OnRecv(endpoint_it->second.remote_ip_addr, endpoint_it->second.remote_port, endpoint_it->second.local_port, pIoData->buffer, pIoData->bytesTransferred);
      }
			
      ZeroMemory(&(pIoData->overlapped), sizeof(pIoData->overlapped));
      pIoData->databuff.len = MAX_SOCKET_MSG_BUFF_LENGTH;
      pIoData->databuff.buf = pIoData->buffer;
      pIoData->bytesTransferred = 0;
      recvBytes = bytesTransferred;

      flags = 0;
      if (WSARecv(pHandleData->socket, &(pIoData->databuff), 1, &recvBytes, &flags, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
      {
        if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::stringstream ss;
			ss << "SocketAccept::ServerWorkThread WSARecv() failed. Error:" << GetLastError();
			DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
          return false;
        }
      }
			

      //GlobalFree(pHandleData);
      //GlobalFree(pIoData);
    }
  } // while end
}

bool SocketAccept::Listen(Port port)
{
  if (listening_)
    return false;

  HANDLE completionPort;
  LPPER_HANDLE_DATA pHandleData;
  LPPER_IO_OPERATION_DATA pIoData;
  DWORD recvBytes;
  DWORD flags;

  WSADATA wsaData;
  DWORD ret;
  if (ret = WSAStartup(0x0202, &wsaData) != 0)
  {
	  std::stringstream ss;
	  ss << "SocketAccept::Listen WSAStartup failed. Error:" << ret;
	  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
    return false;
  }

  completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
  if (NULL == completionPort)
  {
	  std::stringstream ss;
	  ss << "SocketAccept::Listen CreateIoCompletionPort failed. Error:" << GetLastError();
	  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
    return false;
  }

  SYSTEM_INFO mySysInfo;
  GetSystemInfo(&mySysInfo);

  DWORD threadID;
  for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2 + 1); ++i)
  {
    HANDLE threadHandle;
    threadHandle = CreateThread(NULL, 0, SocketAccept::ServerWorkThread, completionPort, 0, &threadID);
    if (NULL == threadHandle)
	{
		std::stringstream ss;
		ss << "SocketAccept::Listen CreateThread failed. Error:" << GetLastError();
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
      return false;
    }

    CloseHandle(threadHandle);
  }

  // 启动一个监听socket
  SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (listenSocket == INVALID_SOCKET)
  {
	  std::stringstream ss;
	  ss << "SocketAccept::Listen WSASocket failed. Error:" << GetLastError();
	  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
    return false;
  }

  SOCKADDR_IN internetAddr;
  internetAddr.sin_family = AF_INET;
  internetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  internetAddr.sin_port = htons(port);

  // 绑定监听端口
  if (bind(listenSocket, (PSOCKADDR)&internetAddr, sizeof(internetAddr)) == SOCKET_ERROR)
  {
	  std::stringstream ss;
	  ss << "SocketAccept::Listen Bind failed. Error:" << GetLastError();
	  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
    return false;
  }

  if (listen(listenSocket, MAX_BACKLOG) == SOCKET_ERROR)
  {
	  std::stringstream ss;
	  ss << "SocketAccept::Listen listen failed. Error:" << GetLastError();
	  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
    return false;
  }

  local_port_ = port;
  listening_ = true;

  // 开始死循环，处理数据
  while (true)
  {
    SOCKET acceptSocket;
    SOCKADDR_IN client_sock_addr;
    int addr_size = sizeof(client_sock_addr);
    acceptSocket = WSAAccept(listenSocket, (sockaddr *)&client_sock_addr, &addr_size, NULL, 0);
    if (acceptSocket == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "SocketAccept::Listen WSAAccept failed. Error:" << GetLastError();
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
      return false;
    }

    pHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
    if (NULL == pHandleData)
	{
		std::stringstream ss;
		ss << "SocketAccept::Listen GlobalAlloc(pHandleData) failed. Error:" << GetLastError();
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
      return false;
    }

    pHandleData->socket = acceptSocket;
    if (NULL == CreateIoCompletionPort((HANDLE)acceptSocket, completionPort, (ULONG_PTR)pHandleData, 0))
	{
		std::stringstream ss;
		ss << "SocketAccept::Listen CreateIoCompletionPort failed. Error:" << GetLastError();
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
      return false;
    }

    pIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
    if (NULL == pIoData)
	{
		std::stringstream ss;
		ss << "SocketAccept::Listen GlobalAlloc(IoData) failed. Error:" << GetLastError();
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
      return false;
    }

    ZeroMemory(&(pIoData->overlapped), sizeof(pIoData->overlapped));
    pIoData->databuff.len = MAX_SOCKET_MSG_BUFF_LENGTH;
    pIoData->databuff.buf = pIoData->buffer;
    pIoData->type = IOCPHandleType::RECV;
    pIoData->bytesTransferred = 0;
    pIoData->accept_ptr = this;

    flags = 0;
    if (WSARecv(acceptSocket, &(pIoData->databuff), 1, &recvBytes, &flags, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
    {
      if (WSAGetLastError() != ERROR_IO_PENDING)
	  {
		  std::stringstream ss;
		  ss << "SocketAccept::Listen WSARecv failed. Error:" << GetLastError();
		  DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
        return false;
      }
      else
      {
        //std::cout << "WSARecv() io pending" << std::endl;
        //return false;
      }
    }

    char ip_buff[20];
    inet_ntop(AF_INET, &client_sock_addr.sin_addr, ip_buff, 20);
    Endpoint cur_endpoint(ip_buff, client_sock_addr.sin_port, port);
    //Endpoint cur_endpoint(inet_ntoa(client_sock_addr.sin_addr), client_sock_addr.sin_port);

    sock_endpoint_map_[acceptSocket] = cur_endpoint;
    endpoint_sock_map_[cur_endpoint] = acceptSocket;

    if (nullptr != handler_)
      handler_->OnAccept(cur_endpoint.remote_ip_addr, cur_endpoint.remote_port, cur_endpoint.local_port);
  }

  local_port_ = 0;
  CloseHandle(completionPort);
  listening_ = false;
  return true;
}

bool SocketAccept::Write(IPAddr ip, Port port, const char *data, int length)
{
	if (nullptr == data || length <= 0)
	{
		std::stringstream ss;
		ss << "SocketAccept::Write Error: data null or length is 0";
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
		return false;
	}

	if (length > MAX_SOCKET_MSG_BUFF_LENGTH)
	{
		std::stringstream ss;
		ss << "SocketAccept::Write Error : length(" << length<<") > " << MAX_SOCKET_MSG_BUFF_LENGTH;
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
		return false;
	}

  auto sock_it = endpoint_sock_map_.find(Endpoint(ip, port, local_port_));
  if (sock_it == endpoint_sock_map_.end())
    return false;

  SOCKET cur_sock = sock_it->second;
  DWORD recvBytes = 0;

  LPPER_IO_OPERATION_DATA pIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
  if (NULL == pIoData)
    return false;
  ZeroMemory(&(pIoData->overlapped), sizeof(pIoData->overlapped));
  memcpy(pIoData->buffer, data, length);
  pIoData->databuff.len = length;
  pIoData->databuff.buf = pIoData->buffer;
  pIoData->type = IOCPHandleType::SEND;
  pIoData->accept_ptr = this;

  if (!send_queue_map_[cur_sock].empty()) // aleady pending, must wait for send complete
  {
    send_queue_map_[cur_sock].push(pIoData);
  }
  else if (WSASend(cur_sock, &(pIoData->databuff), 1, &recvBytes, 0, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
  {
    if (WSAGetLastError() != ERROR_IO_PENDING)
	{
		std::stringstream ss;
		ss << "SocketAccept::Write WSASend() failed.Error:" << GetLastError();
		DebugMessage::GetInstance().ShowMessage(DebugMessageType::BASE_NETWORK, ss.str());
      return false;
    }
    else
    {
      send_queue_map_[cur_sock].push(NULL);
      //std::cout << "WSASend() io pending" << std::endl;
      //return false;
    }
  }
  return true;
}

bool SocketAccept::Disconnect(IPAddr ip, Port port)
{
  auto sock_it = endpoint_sock_map_.find(Endpoint(ip, port, local_port_));
  if (sock_it == endpoint_sock_map_.end())
    return false;

  SOCKET cur_sock = sock_it->second;
  closesocket(cur_sock);

  return true;
}

#endif

bool SocketAccept::CheckOnHandle(IPAddr ip, Port port)
{
	return endpoint_sock_map_.find(Endpoint(ip, port, local_port_)) != endpoint_sock_map_.end();
}

}
