#pragma once

#include <elegance/memory/byte_array/byte_array.hpp>
#include <map>

namespace face2wind {

const static int SERIALIZE_CLASS_NAME_MAX_LEN = 256;

class SerializeNetworkManager;

class SerializeBase
{
 public:
  SerializeBase() {}
  virtual ~SerializeBase() {}
  
  virtual const std::string GetTypeName() const = 0;
  virtual void Serialize(ByteArray &collector) const = 0;
  virtual void Unserialize(ByteArray &collector) = 0;
};

class SerializeDescribe
{
 public:
  SerializeDescribe() {}
  virtual ~SerializeDescribe() {}

  static SerializeBase * CreateSerialize(std::string class_name);

 protected:
  virtual SerializeBase * CreateSerialize() const = 0;

  static std::map<std::string, SerializeDescribe*> & GetNameToObjectMap();
};

}
