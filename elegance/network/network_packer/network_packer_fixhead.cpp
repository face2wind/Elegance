#include <elegance/network/network_packer/network_packer_fixhead.hpp>
#include <iostream>

namespace face2wind
{
void NetworkPackagerFixHead::UnPack(NetworkID net_id, char *data, int length)
{
  auto buffer_it = net_id_to_buffer_map_.find(net_id);
  if (buffer_it != net_id_to_buffer_map_.end() && buffer_it->second.cur_read_size >= NET_PACKER_FIXHEAD_HEADER_LENGTH) // read body
  {
    NetPackerFixHeadMsgHeader *header = (NetPackerFixHeadMsgHeader*)buffer_it->second.buffer.buffer;
    int left_bytes = NET_PACKER_FIXHEAD_HEADER_LENGTH + header->package_length - buffer_it->second.cur_read_size;

    int copy_bytes = length;
    if (length >= left_bytes)
      copy_bytes = left_bytes;

    memcpy(buffer_it->second.buffer.buffer + buffer_it->second.cur_read_size, data, copy_bytes);
    buffer_it->second.cur_read_size += copy_bytes;

    // read package complete
    if (buffer_it->second.cur_read_size == NET_PACKER_FIXHEAD_HEADER_LENGTH + header->package_length)
    {
      this->ManagerRecv(net_id, buffer_it->second.buffer.buffer + NET_PACKER_FIXHEAD_HEADER_LENGTH, header->package_length);
      buffer_it->second.buffer.ZeroAllBytes();
      buffer_it->second.cur_read_size = 0;
    }

    int remain_bytes = length - copy_bytes;
    if (remain_bytes > 0)
      this->UnPack(net_id, data + copy_bytes, remain_bytes);
  }
  else // read header
  {
    if (buffer_it == net_id_to_buffer_map_.end()) // first read, create buffer
    {
      net_id_to_buffer_map_[net_id].cur_read_size = 0;
      net_id_to_buffer_map_[net_id].buffer.ChangeBufferSize(NET_PACKER_FIXHEAD_HEADER_LENGTH);

      buffer_it = net_id_to_buffer_map_.find(net_id);
      if (buffer_it == net_id_to_buffer_map_.end())
        return;
    }

    int left_bytes = NET_PACKER_FIXHEAD_HEADER_LENGTH - buffer_it->second.cur_read_size;

    int copy_bytes = length;
    if (length >= left_bytes)
      copy_bytes = left_bytes;

    memcpy(buffer_it->second.buffer.buffer + buffer_it->second.cur_read_size, data, copy_bytes);
    buffer_it->second.cur_read_size += copy_bytes;

    if (buffer_it->second.cur_read_size >= NET_PACKER_FIXHEAD_HEADER_LENGTH) // header read complete
    {
      NetPackerFixHeadMsgHeader *header = (NetPackerFixHeadMsgHeader*)buffer_it->second.buffer.buffer;
      int all_pack_size = NET_PACKER_FIXHEAD_HEADER_LENGTH + header->package_length;

      if (buffer_it->second.buffer.buffer_size < all_pack_size)
        buffer_it->second.buffer.ChangeBufferSize(all_pack_size);
    }

    int remain_bytes = length - copy_bytes;
    if (remain_bytes > 0)
      this->UnPack(net_id, data + copy_bytes, remain_bytes);
  }
}

void NetworkPackagerFixHead::PackAndSend(NetworkID net_id, const char *data, int length)
{
  NetPackerFixHeadMsgHeader header;
  header.package_length = length;

  FlexibleBuffer buffer;
  buffer.ChangeBufferSize(NET_PACKER_FIXHEAD_HEADER_LENGTH + length);

  memcpy(buffer.buffer, (char *)&header, NET_PACKER_FIXHEAD_HEADER_LENGTH);
  memcpy(buffer.buffer + NET_PACKER_FIXHEAD_HEADER_LENGTH, data, length);

  this->ManagerSend(net_id, buffer.buffer, buffer.buffer_size);
}
}
