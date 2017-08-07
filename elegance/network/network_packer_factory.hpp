#pragma once

#include <elegance/platform/network/socket_def.hpp>

namespace face2wind {

	class NetworkManager;

	class INetworkPackager
	{
	public:
		INetworkPackager(NetworkManager *mgr) : net_manager_(mgr) {}
		virtual ~INetworkPackager() {}

		virtual void UnPack(NetworkID net_id, char *data, int length) = 0;
		virtual void PackAndSend(NetworkID net_id, const char *data, int length) = 0;

	protected:
		void ManagerSend(NetworkID net_id, const char *data, int length);
		void ManagerRecv(NetworkID net_id, char *data, int length);

		NetworkManager *net_manager_;
	};

	enum class NetworkPackerType
	{
		BEGIN = 0,
		FIX_HEAD = BEGIN,
	};

	class NetworkPackerFactory
	{
	public:
		static INetworkPackager *CreatePacker(NetworkPackerType type, NetworkManager *mgr);
	};

}
