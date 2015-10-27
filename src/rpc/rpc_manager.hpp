#ifndef __RPC_MANAGER_HPP__
#define __RPC_MANAGER_HPP__

#include <string>
#include "network/i_network.hpp"

namespace face2wind
{
	class IRPCHandler
	{
	public:
		virtual void OnConnected(const std::string &server_ip, Port port, bool success) = 0;
		virtual void OnListened(const std::string &server_ip, Port port, bool success) = 0;
	};

	class IRPCSeccsion

	class RPCManager
	{
	public:
		void Connect(const std::string &server_ip, Port port, const std::string key);
		void Listen(Port port, const std::string key);

		void Call();
	};
};

#endif