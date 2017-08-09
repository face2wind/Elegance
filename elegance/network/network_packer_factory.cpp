#include <elegance/network/network_packer_factory.hpp>
#include <elegance/network/network_manager.hpp>

#include <elegance/network/network_packer/network_packer_fixhead.hpp>

namespace face2wind
{
	void INetworkPackager::ManagerSend(NetworkID net_id, const char *data, int length)
	{
		if (nullptr == net_manager_)
			return;

		net_manager_->SendRaw(net_id, data, length);
	}

	void INetworkPackager::ManagerRecv(NetworkID net_id, char *data, int length)
	{
		if (nullptr == net_manager_)
			return;

		net_manager_->OnRecvPackage(net_id, data, length);
	}

	INetworkPackager *NetworkPackerFactory::CreatePacker(NetworkPackerType type, NetworkManager *mgr)
	{
		INetworkPackager *packer = nullptr;

		switch (type)
		{
		case NetworkPackerType::FIX_HEAD:
			packer = new NetworkPackagerFixHead(mgr);
			break;

		default: break;
		}

		return packer;
	}

}
