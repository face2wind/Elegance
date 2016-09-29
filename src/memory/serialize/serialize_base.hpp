#pragma once

#include <memory/byte_array/byte_array.hpp>
#include <map>

namespace face2wind {

const static int SERIALIZE_CLASS_NAME_MAX_LEN = 256;

class SerializeNetworkManager;

class SerializeBase
{
 public:
  SerializeBase() {}
  virtual ~SerializeBase() {}
  
  friend class SerializeNetworkManager;

  static SerializeBase * CreateSerialize(std::string class_name);
  
 protected:
  static std::map<std::string, SerializeBase*> name_to_object_map_;

  virtual SerializeBase * Clone() const = 0;
  virtual const std::string & GetClassName() const = 0;
  virtual void Serialize(ByteArray &collector) const = 0;
  virtual void Unserialize(ByteArray &collector) = 0;
};

}
