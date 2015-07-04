#ifndef _NETWORK_BASIC_HPP_
#define _NETWORK_BASIC_HPP_

#include <string>

namespace face2wind
{
	typedef unsigned int NetworkID;
	typedef unsigned short Port;
	typedef std::string IPAddr;

	class INetworkHandler
	{
	public:
		/*
		当调用异步ConnectAsyn结果返回后回调
		@is_success			连接结果是否成功
		@NetworkID			该连接的NetworkID
		@listen_port		本地端口
		@remote_ip_addr		远端地址，主机序
		@remote_port		远端端口号
		*/
		virtual void OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port) = 0;

		/*
		单有连接accept时则回调该函数
		@NetworkID			该连接的NetworkID
		@listen_port		本地端口
		@remote_ip_addr		远端地址，主机序
		@remote_port		远端端口号
		*/
		virtual void OnAccept(NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port) = 0;

		/*
		当收到网络消息时回调该函数
		@NetworkID			网络消息来源的NetworkID
		@data				网络数据
		@length				数据长度
		*/
		virtual void OnRecv(NetworkID network_id, const char *data, int length) = 0;

		/*
		当有网络断开的时候回调该函数
		@NetworkID			断开的网络连接的NetworkID
		*/
		virtual void OnDisconnect(NetworkID network_id) = 0;
	};

	class IModule
	{
		friend class Game;

	public:
		enum
		{
			Succeed = 0,
			Fail,
			Pending,
		};
		enum State
		{
			ST_Created,
			ST_Inited,
			ST_Started,
			ST_Running,
			ST_Stoped,
			ST_Released,

			ST_Invalid,
		};

		IModule() : m_module_state(ST_Created) {}
		virtual ~IModule(){}

		/*
		 * 初始化数据，读取
		 */
		virtual int Init() = 0;

		/*
		 * 开始运行
		 */
		virtual int Start() = 0;

		/*
		 * 每帧调用一次
		 */
		virtual int Update() = 0;

		/*
		 * 停止运行
		 */
		virtual int Stop() = 0;

		/*
		 * 释放当前数据，持久存储
		 */
		virtual int Release() = 0;

		int GetState() const { return m_module_state;}

	private:
		State m_module_state;

	};
}

#endif