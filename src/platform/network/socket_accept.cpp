#include "socket_accept.hpp"
#include "i_socket_handler.hpp"


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
DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID)
{
	HANDLE complationPort = (HANDLE)CompletionPortID;
	DWORD bytesTransferred;
	LPPER_HANDLE_DATA pHandleData = NULL;
	LPPER_IO_OPERATION_DATA pIoData = NULL;
	DWORD sendBytes = 0;
	DWORD recvBytes = 0;
	DWORD flags;

	while (1)
	{
		if (GetQueuedCompletionStatus(complationPort, &bytesTransferred, (PULONG_PTR)&pHandleData, (LPOVERLAPPED *)&pIoData, INFINITE) == 0)
		{
			std::cout << "GetQueuedCompletionStatus failed. Error:" << GetLastError() << std::endl;
			return 0;
		}

		// 检查数据是否已经传输完了
		if (bytesTransferred == 0)
		{
			std::cout << " Start closing socket..." << std::endl;
			if (CloseHandle((HANDLE)pHandleData->socket) == SOCKET_ERROR)
			{
				std::cout << "Close socket failed. Error:" << GetLastError() << std::endl;
				return 0;
			}

			GlobalFree(pHandleData);
			GlobalFree(pIoData);
			continue;
		}

		// 检查管道里是否有数据
		if (pIoData->bytesRecv == 0)
		{
			pIoData->bytesRecv = bytesTransferred;
			pIoData->bytesSend = 0;
		}
		else
		{
			pIoData->bytesSend += bytesTransferred;
		}

		// 数据没有发完，继续发送
		if (pIoData->bytesRecv > pIoData->bytesSend)
		{
			ZeroMemory(&(pIoData->overlapped), sizeof(OVERLAPPED));
			pIoData->databuff.buf = pIoData->buffer + pIoData->bytesSend;
			pIoData->databuff.len = pIoData->bytesRecv - pIoData->bytesSend;

			// 发送数据出去
			if (WSASend(pHandleData->socket, &(pIoData->databuff), 1, &sendBytes, 0, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					std::cout << "WSASend() failed. Error:" << GetLastError() << std::endl;
					return 0;
				}
				else
				{
					std::cout << "WSASend() failed. io pending. Error:" << GetLastError() << std::endl;
					return 0;
				}
			}

			std::cout << "Send " << pIoData->buffer << std::endl;
		}
		else
		{
			pIoData->bytesRecv = 0;
			flags = 0;

			ZeroMemory(&(pIoData->overlapped), sizeof(OVERLAPPED));
			pIoData->databuff.len = MAX_SOCKET_MSG_BUFF_LENGTH;
			pIoData->databuff.buf = pIoData->buffer;

			if (WSARecv(pHandleData->socket, &(pIoData->databuff), 1, &recvBytes, &flags, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
			{
				if (WSAGetLastError() != ERROR_IO_PENDING)
				{
					std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
					return 0;
				}
				else
				{
					std::cout << "WSARecv() io pending" << std::endl;
					return 0;
				}
			}
		}
	}
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
  if (completionPort == NULL)
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
	  threadHandle = CreateThread(NULL, 0, ServerWorkThread, completionPort, 0, &threadID);
	  if (threadHandle == NULL)
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
  while (1)
  {
	  SOCKET acceptSocket;
	  SOCKADDR_IN client_sock_addr;
	  acceptSocket = WSAAccept(listenSocket, (sockaddr *)&client_sock_addr, NULL, NULL, 0);
	  if (acceptSocket == SOCKET_ERROR)
	  {
		  std::cout << "WSAAccept failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  pHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
	  if (pHandleData = NULL)
	  {
		  std::cout << "GlobalAlloc( HandleData ) failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  pHandleData->socket = acceptSocket;
	  if (CreateIoCompletionPort((HANDLE)acceptSocket, completionPort, (ULONG_PTR)pHandleData, 0) == NULL)
	  {
		  std::cout << "CreateIoCompletionPort failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  pIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
	  if (pIoData == NULL)
	  {
		  std::cout << "GlobalAlloc( IoData ) failed. Error:" << GetLastError() << std::endl;
		  return false;
	  }

	  ZeroMemory(&(pIoData->overlapped), sizeof(pIoData->overlapped));
	  pIoData->bytesSend = 0;
	  pIoData->bytesRecv = 0;
	  pIoData->databuff.len = MAX_SOCKET_MSG_BUFF_LENGTH;
	  pIoData->databuff.buf = pIoData->buffer;

	  if (WSARecv(acceptSocket, &(pIoData->databuff), 1, &recvBytes, &flags, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
	  {
		  if (WSAGetLastError() != ERROR_IO_PENDING)
		  {
			  std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
			  return false;
		  }
		  else
		  {
			  std::cout << "WSARecv() io pending" << std::endl;
			  return false;
		  }
	  }
  }

  listening_ = true;
  return true;
}

bool SocketAccept::Write(IPAddr ip, Port port, const char *data, int length)
{
	if (WSASend(acceptSocket, &(pIoData->databuff), 1, &recvBytes, &flags, &(pIoData->overlapped), NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			std::cout << "WSARecv() failed. Error:" << GetLastError() << std::endl;
			return false;
		}
		else
		{
			std::cout << "WSARecv() io pending" << std::endl;
			return false;
		}
	}

    return true;
}
#endif

}
