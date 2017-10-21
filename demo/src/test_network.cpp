#include <iostream>
#include <elegance/elegance.hpp>

namespace test_network {

	using namespace face2wind;
	
	struct MsgPackage
	{
		short age;
		short reserve_sh;
		char name[32];
	};

	NetworkManager server_net_mgr;
	NetworkManager client_net_mgr;

	class MyNetworkHandlerServer : public INetworkHandler
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

		virtual void OnRecv(NetworkID net_id, const char *data, int length)
		{
			MsgPackage *recv_msg = (MsgPackage *)data;

			std::cout << "[server] Recv { " << recv_msg->age << ", " << recv_msg->name << " } " << std::endl;

			MsgPackage msg;
			msg.age = recv_msg->age + 1;
			
			memcpy(msg.name, recv_msg->name, sizeof(msg.name));
			char tmp_char = msg.name[0];
			msg.name[0] = msg.name[4];
			msg.name[4] = tmp_char;

			server_net_mgr.Send(net_id, (char*)&msg, sizeof(msg));
		}
		virtual void OnDisconnect(NetworkID net_id)
		{
			
		}
	};

	class MyNetworkHandlerClient : public face2wind::INetworkHandler
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

			MsgPackage msg;
			msg.age = 1;
			memcpy(msg.name, "i O u", sizeof(msg.name));
			client_net_mgr.Send(net_id, (char*)&msg, sizeof(msg));
		}

		virtual void OnRecv(NetworkID net_id, const char *data, int length)
		{
			MsgPackage *recv_msg = (MsgPackage *)data;

			std::cout << "[client] Recv { " << recv_msg->age << ", " << recv_msg->name << " } " << std::endl;

			MsgPackage msg;
			msg.age = recv_msg->age + 1;

			memcpy(msg.name, recv_msg->name, sizeof(msg.name));
			char tmp_char = msg.name[0];
			msg.name[0] = msg.name[4];
			msg.name[4] = tmp_char;

			client_net_mgr.Send(net_id, (char*)&msg, sizeof(msg));

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
			client_net_mgr.RegistHandler(new MyNetworkHandlerClient());
			client_net_mgr.SyncConnect("127.0.0.1", 33022);
			client_net_mgr.SyncRunning();
			//client_net_mgr.WaitAllThread();
		}
	};
}

using namespace test_network;

int TestNetwork()
{
	MyNetworkHandlerServer *server_net_handler = new MyNetworkHandlerServer();
	server_net_mgr.RegistHandler(server_net_handler);
	server_net_mgr.SyncListen(33022);

	Thread *t = new Thread();
	t->Run(new MyThreadTask());

	server_net_mgr.SyncRunning();
	//server_net_mgr.WaitAllThread();
	t->Join();
	delete t;

	return 0;
}
