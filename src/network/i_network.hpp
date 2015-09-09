#ifndef _I_NETWORK_HPP_
#define _I_NETWORK_HPP_

#include <string>

namespace face2wind
{
	typedef unsigned int NetworkID;
	typedef unsigned short Port;
	typedef std::string IPAddr;

	class INetworkHandler
	{
	public:
		INetworkHandler(){}
		virtual ~INetworkHandler(){}

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
}

#endif