#include "socket_accept.hpp"
#include "i_socket_handler.hpp"


namespace face2wind {

void SocketAccept::ResetHandler(ISocketHandler *handler)
{
  handler_ = handler;
}

#ifdef __LINUX__
SocketAccept::SocketAccept() : handler_(NULL), listening_(false)
{
}

SocketAccept::~SocketAccept()
{
}

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
  
  while(true)
  {
    //std::cout<<"[server] start epoll wait ..."<<std::endl;
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
        
        //std::cout<<"[server] accept succ from - "<<cur_endpoint.ip_addr<<":"<<cur_endpoint.port<<std::endl;
        if (nullptr != handler_)
          handler_->OnAccept(cur_endpoint.ip_addr, cur_endpoint.port);
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
              handler_->OnRecv(endpoint_it->second.ip_addr, endpoint_it->second.port, buff_, read_size);
            
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
            handler_->OnDisconnect(endpoint_it->second.ip_addr, endpoint_it->second.port);
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
          handler_->OnDisconnect(endpoint_it->second.ip_addr, endpoint_it->second.port);
        if (endpoint_it != sock_endpoint_map_.end())
        {
          endpoint_sock_map_.erase(endpoint_it->second);
          sock_endpoint_map_.erase(endpoint_it);
        }
      }
    }
  }

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
#endif

#ifdef __WINDOWS__
SocketAccept::SocketAccept() : handler_(NULL), listening_(false), all_send_(true)
{
}

SocketAccept::~SocketAccept()
{
}

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
		if (GetQueuedCompletionStatus(complationPort, &bytesTransferred, (PULONG_PTR)&pHandleData, (LPOVERLAPPED *)&pIoData, INFINITE) == 0)
		{
			std::cout << "GetQueuedCompletionStatus failed. Error:" << GetLastError() << std::endl;
			return 0;
		}

		if (bytesTransferred == 0)
		{
			// socket aleady disconnect
			
			if (nullptr != pIoData->accept_ptr)
			{
				auto endpoint_it = pIoData->accept_ptr->sock_endpoint_map_.find(pHandleData->socket);

				if (nullptr != pIoData->accept_ptr->handler_ && endpoint_it != pIoData->accept_ptr->sock_endpoint_map_.end())
					pIoData->accept_ptr->handler_->OnDisconnect(endpoint_it->second.ip_addr, endpoint_it->second.port);

				pIoData->accept_ptr->endpoint_sock_map_.erase(endpoint_it->second);
				pIoData->accept_ptr->sock_endpoint_map_.erase(endpoint_it);
				pIoData->accept_ptr->send_queue_map_.erase(pHandleData->socket);
			}

			if (CloseHandle((HANDLE)pHandleData->socket) == SOCKET_ERROR)
			{
				std::cout << "Close socket failed. Error:" << GetLastError() << std::endl;
				return 0;
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
								std::cout << "WSASend() failed. Error:" << GetLastError() << std::endl;
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
						std::cout << "WSASend() failed. Error:" << GetLastError() << std::endl;
						return 0;
					}
				}
				std::cout << "Send " << pIoData->buffer << std::endl;
			}
		}
		else if (pIoData->type == IOCPHandleType::RECV)
		{
			// 接收数据，不管有没有接收完全，都直接触发事件，并重新接收下一段数据

			if (nullptr != pIoData->accept_ptr)
			{
				auto endpoint_it = pIoData->accept_ptr->sock_endpoint_map_.find(pHandleData->socket);

				if (nullptr != pIoData->accept_ptr->handler_ && endpoint_it != pIoData->accept_ptr->sock_endpoint_map_.end())
					pIoData->accept_ptr->handler_->OnRecv(endpoint_it->second.ip_addr, endpoint_it->second.port, pIoData->buffer, pIoData->bytesTransferred);
			}

			LPPER_IO_OPERATION_DATA new_pIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
			if (NULL == new_pIoData)
			{
				std::cout << "GlobalAlloc( IoData ) failed. Error:" << GetLastError() << std::endl;
				return false;
			}

			ZeroMemory(&(new_pIoData->overlapped), sizeof(new_pIoData->overlapped));
			new_pIoData->databuff.len = MAX_SOCKET_MSG_BUFF_LENGTH;
			new_pIoData->databuff.buf = new_pIoData->buffer;
			new_pIoData->type = IOCPHandleType::RECV;
			new_pIoData->bytesTransferred = 0;
			new_pIoData->accept_ptr = pIoData->accept_ptr;

			flags = 0;
			if (WSARecv(pHandleData->socket, &(new_pIoData->databuff), 1, &recvBytes, &flags, &(new_pIoData->overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
					return false;
				}
			}

			/*
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
					std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
					return false;
				}
			}
			*/

			//GlobalFree(pHandleData);
			GlobalFree(pIoData);
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
	  std::cout << "WSAStartup failed. Error:" << ret << std::endl;
	  return false;
  }

  completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
  if (NULL == completionPort)
  {
	  std::cout << "CreateIoCompletionPort failed. Error:" << GetLastError() << std::endl;
	  return false;
  }

  SYSTEM_INFO mySysInfo;
  GetSystemInfo(&mySysInfo);

  // 创建 2 * CPU核数 + 1 个线程
  DWORD threadID;
  for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2 + 1); ++i)
  {
	  HANDLE threadHandle;
	  threadHandle = CreateThread(NULL, 0, SocketAccept::ServerWorkThread, completionPort, 0, &threadID);
	  if (NULL == threadHandle)
	  {
		  std::cout << "CreateThread failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  CloseHandle(threadHandle);
  }

  // 启动一个监听socket
  SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
  if (listenSocket == INVALID_SOCKET)
  {
	  std::cout << " WSASocket( listenSocket ) failed. Error:" << GetLastError() << std::endl;
	  return false;
  }

  SOCKADDR_IN internetAddr;
  internetAddr.sin_family = AF_INET;
  internetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  internetAddr.sin_port = htons(port);

  // 绑定监听端口
  if (bind(listenSocket, (PSOCKADDR)&internetAddr, sizeof(internetAddr)) == SOCKET_ERROR)
  {
	  std::cout << "Bind failed. Error:" << GetLastError() << std::endl;
	  return false;
  }

  if (listen(listenSocket, MAX_BACKLOG) == SOCKET_ERROR)
  {
	  std::cout << "listen failed. Error:" << GetLastError() << std::endl;
	  return false;
  }

  // 开始死循环，处理数据
  while (true)
  {
	  SOCKET acceptSocket;
	  SOCKADDR_IN client_sock_addr;
	  int addr_size = sizeof(client_sock_addr);
	  acceptSocket = WSAAccept(listenSocket, (sockaddr *)&client_sock_addr, &addr_size, NULL, 0);
	  if (acceptSocket == SOCKET_ERROR)
	  {
		  std::cout << "WSAAccept failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  pHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
	  if (NULL == pHandleData)
	  {
		  std::cout << "GlobalAlloc( HandleData ) failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  pHandleData->socket = acceptSocket;
	  if (NULL == CreateIoCompletionPort((HANDLE)acceptSocket, completionPort, (ULONG_PTR)pHandleData, 0))
	  {
		  std::cout << "CreateIoCompletionPort failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  pIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
	  if (NULL == pIoData)
	  {
		  std::cout << "GlobalAlloc( IoData ) failed. Error:" << GetLastError() << std::endl;
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
			  std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
			  return false;
		  }
		  else
		  {
			  //std::cout << "WSARecv() io pending" << std::endl;
			  //return false;
		  }
	  }

	  Endpoint cur_endpoint(inet_ntoa(client_sock_addr.sin_addr), client_sock_addr.sin_port);

	  sock_endpoint_map_[acceptSocket] = cur_endpoint;
	  endpoint_sock_map_[cur_endpoint] = acceptSocket;

	  if (nullptr != handler_)
		  handler_->OnAccept(cur_endpoint.ip_addr, cur_endpoint.port);
  }

  listening_ = true;
  return true;
}

bool SocketAccept::Write(IPAddr ip, Port port, const char *data, int length)
{
	if (nullptr == data || length <= 0 || length > MAX_SOCKET_MSG_BUFF_LENGTH)
		return false;

	auto sock_it = endpoint_sock_map_.find(Endpoint(ip, port));
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
			std::cout << "WSASend() failed. Error:" << GetLastError() << std::endl;
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
#endif

}
