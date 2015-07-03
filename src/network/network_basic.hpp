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
		�������첽ConnectAsyn������غ�ص�
		@result		���ӽ���Ƿ�ɹ�
		@handle		ConnectAsynʱ���������handle
		@NetworkID			���ӳɹ�ʱ��NetworkIDʱ�������ӵ������id
		@ip_addr			����ip_addr
		@port			����port
		*/
		virtual void OnConnect(bool result, int handle, NetworkID network_id, IPAddr ip_addr, Port port) = 0;

		/*
		��������acceptʱ��ص��ú���
		@NetworkID			�����ӵ�NetworkID
		@ip_addr			Զ�˵�ַ��������
		@port			Զ�˶˿ں�
		*/
		virtual void OnAccept(Port listen_port, NetworkID network_id, IPAddr ip_addr, Port port) = 0;

		/*
		���յ�������Ϣʱ�ص��ú���
		@NetworkID			������Ϣ��Դ��NetworkID
		@data			��������
		@length		���ݳ���
		*/
		virtual void OnRecv(NetworkID network_id, const char *data, int length) = 0;

		/*
		��������Ͽ���ʱ��ص��ú���
		@NetworkID			�Ͽ����������ӵ�NetworkID
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
		 * ��ʼ�����ݣ���ȡ
		 */
		virtual int Init() = 0;

		/*
		 * ��ʼ����
		 */
		virtual int Start() = 0;

		/*
		 * ÿ֡����һ��
		 */
		virtual int Update() = 0;

		/*
		 * ֹͣ����
		 */
		virtual int Stop() = 0;

		/*
		 * �ͷŵ�ǰ���ݣ��־ô洢
		 */
		virtual int Release() = 0;

		int GetState() const { return m_module_state;}

	private:
		State m_module_state;

	};
}

#endif