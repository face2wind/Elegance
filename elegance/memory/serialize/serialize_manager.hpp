#pragma once

#include "serialize_base.hpp"
#include "elegance/network/network_manager.hpp"

namespace face2wind {

class ISerializeNetworkHandler : public INetworkHandler
{
 public:
  virtual void OnListenFail(Port port) = 0;
  virtual void OnAccept(IPAddr ip, Port port, Port local_port, NetworkID net_id) = 0;
  virtual void OnConnect(IPAddr ip, Port port, Port local_port, bool success, NetworkID net_id) = 0;

  virtual void OnRecv(NetworkID net_id, const char *data, int length) {}
  virtual void OnRecv(NetworkID net_id, const SerializeBase *data) = 0;
  virtual void OnDisconnect(NetworkID net_id) = 0;
};

class SerializeNetworkManager : public NetworkManager
{
 public:
  void SendSerialize(NetworkID net_id, const SerializeBase &data);

  void RegistSerializeHandler(ISerializeNetworkHandler *handler);
  void UnregistSerializeHandler(ISerializeNetworkHandler *handler);

 protected:
  virtual void OnRecvPackage(NetworkID net_id, char *data, int length);

 private:
  std::set<ISerializeNetworkHandler *> serialize_handler_list_;
};

}

