#include "serialize_base.hpp"

namespace face2wind {

std::map<std::string, SerializeDescribe*> SerializeDescribe::name_to_object_map_;

SerializeBase * SerializeDescribe::CreateSerialize(std::string class_name)
{
  auto obj_it = name_to_object_map_.find(class_name);
  if (obj_it == name_to_object_map_.end())
    return nullptr;
  
  const SerializeDescribe *object = obj_it->second;
  return object->CreateSerialize();
}

}
