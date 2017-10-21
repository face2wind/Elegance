#include <iostream>
#include <elegance/elegance.hpp>
#include "serialize_demo/test_serialize.hpp"

namespace test_serilaize_network {

	using namespace face2wind;
	
	struct MsgPackage
	{
		short age;
		short reserve_sh;
		char name[32];
	};

	SerializeNetworkManager server_net_mgr;
	SerializeNetworkManager client_net_mgr;

	class MyNetworkHandlerServer : public ISerializeNetworkHandler
	{
	public:
		virtual void OnListenFail(Port port)
		{
			
		}
		virtual void OnAccept(IPAddr ip, Port port, Port local_port, NetworkID net_id)
		{
			//std::cout << "[server] OnAccept(" << ip << ", " << port << ", " << local_port << ", "<< net_id<<")"<<std::endl;
		}
		virtual void OnConnect(IPAddr ip, Port port, Port local_port, bool success, NetworkID net_id) {}

		virtual void OnRecv(NetworkID net_id, const SerializeBase *data)
		{
			Protocol::CSCheckServiceInfo *recv_msg = (Protocol::CSCheckServiceInfo *)data;

			std::cout << "[server] Recv { service_type = " << recv_msg->service_type << " } " << std::endl;

			Protocol::SCCheckServiceInfoAck msg;
			msg.service_type = recv_msg->service_type + 1;
			msg.ip_addr = "123.188.111.1";
			msg.port = 52013;

			server_net_mgr.SendSerialize(net_id, msg);
		}
		virtual void OnDisconnect(NetworkID net_id)
		{
			
		}
	};

	class MyNetworkHandlerClient : public face2wind::ISerializeNetworkHandler
	{
	public:
		virtual void OnListenFail(Port port) {}
		virtual void OnAccept(IPAddr ip, Port port, Port local_port, NetworkID net_id) {}
		virtual void OnConnect(IPAddr ip, Port port, Port local_port, bool success, NetworkID net_id)
		{
			std::cout << "[client] OnConnect(" << ip << ", " << port << ", " << local_port << ", ";
			if (success)
				std::cout << ") Success , NetworkID ="<<net_id<<")" << std::endl;
			else
				std::cout << ") Fail" << std::endl;

			Protocol::CSCheckServiceInfo msg;
			msg.service_type = 1;
			client_net_mgr.SendSerialize(net_id, msg);
		}

		virtual void OnRecv(NetworkID net_id, const SerializeBase *data)
		{
			Protocol::SCCheckServiceInfoAck *recv_msg = (Protocol::SCCheckServiceInfoAck *)data;

			//std::cout << "[client] Recv { service_type = " << recv_msg->service_type << ", ip_addr = " << recv_msg->ip_addr << ", port = " << recv_msg->port << " } " << std::endl;

			Protocol::CSCheckServiceInfo msg;
			msg.service_type = recv_msg->service_type + 1;
			client_net_mgr.SendSerialize(net_id, msg);
			//Timer::Sleep(1000);
		}
		virtual void OnDisconnect(NetworkID net_id)
		{
			
		}
	};

	class MyThreadTask : public IThreadTask
	{
		virtual void Run()
		{
			client_net_mgr.RegistSerializeHandler(new MyNetworkHandlerClient());
			client_net_mgr.SyncConnect("127.0.0.1", 33022);
			client_net_mgr.SyncRunning();
			//client_net_mgr.WaitAllThread();
		}
	};
}

using namespace test_serilaize_network;

int TestSerilaizeNetwork()
{
	MyNetworkHandlerServer *server_net_handler = new MyNetworkHandlerServer();
	server_net_mgr.RegistSerializeHandler(server_net_handler);
	server_net_mgr.SyncListen(33022);

	Thread *t = new Thread();
	t->Run(new MyThreadTask());

	server_net_mgr.SyncRunning();
	//server_net_mgr.WaitAllThread();
	t->Join();
	delete t;

	return 0;
}
