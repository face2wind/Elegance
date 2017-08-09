#include "serialize_manager.hpp"
#include "elegance/memory/byte_array/queue_byte_array.hpp"
#include "elegance/network/network_manager.hpp"

#include <cstring>

namespace face2wind {

void SerializeNetworkManager::SendSerialize(NetworkID net_id, const SerializeBase &data)
{
  QueueByteArray by;

  by.WriteString(data.GetTypeName());
  data.Serialize(by);

  int len = by.BytesAvailable();
  char *char_data = (char*)by.ReadObject(len);
  if (nullptr == char_data)
    return;
  
  NetworkManager::Send(net_id, char_data, len);
  delete []char_data;
}

void SerializeNetworkManager::RegistSerializeHandler(ISerializeNetworkHandler *handler)
{
  this->RegistHandler(handler);
  serialize_handler_list_.insert(handler);
}

void SerializeNetworkManager::UnregistSerializeHandler(ISerializeNetworkHandler *handler)
{
  this->UnregistHandler(handler);
  serialize_handler_list_.erase(handler);
}

void SerializeNetworkManager::OnRecvPackage(NetworkID net_id, char *data, int length)
{
  NetworkManager::OnRecvPackage(net_id, data, length);

  static char serialize_name[SERIALIZE_CLASS_NAME_MAX_LEN];
  short *s_name_len = (short*)data;
  if (*s_name_len <= 0 || *s_name_len >= SERIALIZE_CLASS_NAME_MAX_LEN)
    return;
  
  memcpy(serialize_name, data + sizeof(short), *s_name_len);
  serialize_name[*s_name_len] = '\0';
    
  SerializeBase *serialize_data = SerializeDescribe::CreateSerialize(serialize_name);
  if (nullptr == serialize_data)
    return;

  int serialize_len = length - sizeof(short) - *s_name_len;
  if (serialize_len < 0)
    return;
  
  QueueByteArray by;
  by.WriteObject(data + length - serialize_len, serialize_len);
  serialize_data->Unserialize(by);
  
  for (auto handler_ptr : serialize_handler_list_)
    handler_ptr->OnRecv(net_id, serialize_data);

  delete serialize_data;
}

}

