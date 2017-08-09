#pragma once

#include <elegance/network/network_packer_factory.hpp>
#include <elegance/memory/flexible_buffer.hpp>

#include <map>

namespace face2wind {


#pragma pack(push, 4)

struct NetPackerFixHeadMsgHeader
{
  int package_length;
};
#pragma pack(pop)

const static int NET_PACKER_FIXHEAD_HEADER_LENGTH = sizeof(NetPackerFixHeadMsgHeader);

struct NetPackerFixHeadMsgCacheBuffer
{
  NetPackerFixHeadMsgCacheBuffer() : cur_read_size(0) {}
  ~NetPackerFixHeadMsgCacheBuffer() {}

  FlexibleBuffer buffer;
  int cur_read_size;
};

class NetworkPackagerFixHead : public INetworkPackager
{
 public:
  NetworkPackagerFixHead(NetworkManager *mgr) : INetworkPackager(mgr) {}
  virtual ~NetworkPackagerFixHead() {}

  virtual void UnPack(NetworkID net_id, char *data, int length);
  virtual void PackAndSend(NetworkID net_id, const char *data, int length);

 protected:
  std::map<NetworkID, NetPackerFixHeadMsgCacheBuffer> net_id_to_buffer_map_;
};

}
