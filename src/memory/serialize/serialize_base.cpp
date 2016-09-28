#include "serialize_base.hpp"

namespace face2wind {

SerializeBase * SerializeBase::CreateSerialize(std::string class_name)
{
  auto obj_it = name_to_object_map_.find(class_name);
  if (obj_it == name_to_object_map_.end())
    return nullptr;
  
  const SerializeBase *object = obj_it->second;
  return object->Clone();
}

}
