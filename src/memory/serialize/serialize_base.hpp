#pragma once

#include <memory/byte_array/byte_array.hpp>

namespace face2wind {

class SerializeNetworkManager;

class SerializeBase
{
 public:
  friend class SerializeNetworkManager;
  
 protected:
  virtual void Serialize(ByteArray &collector) = 0;
  virtual void Unserialize(ByteArray &collector) = 0;
};

}
