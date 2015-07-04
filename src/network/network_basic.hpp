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
		@is_success			���ӽ���Ƿ�ɹ�
		@NetworkID			�����ӵ�NetworkID
		@listen_port		���ض˿�
		@remote_ip_addr		Զ�˵�ַ��������
		@remote_port		Զ�˶˿ں�
		*/
		virtual void OnConnect(bool is_success, NetworkID network_id, Port local_port, IPAddr remote_ip_addr, Port remote_port) = 0;

		/*
		��������acceptʱ��ص��ú���
		@NetworkID			�����ӵ�NetworkID
		@listen_port		���ض˿�
		@remote_ip_addr		Զ�˵�ַ��������
		@remote_port		Զ�˶˿ں�
		*/
		virtual void OnAccept(NetworkID network_id, Port listen_port, IPAddr remote_ip_addr, Port remote_port) = 0;

		/*
		���յ�������Ϣʱ�ص��ú���
		@NetworkID			������Ϣ��Դ��NetworkID
		@data				��������
		@length				���ݳ���
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