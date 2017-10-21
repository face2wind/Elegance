#include "test_serialize.hpp"

namespace Protocol {

__CSCheckServiceInfoDescribe__ for_describe_register_to___cscheckserviceinfodescribe__;
__SCCheckServiceInfoAckDescribe__ for_describe_register_to___sccheckserviceinfoackdescribe__;
__CSGORequestCommandListDescribe__ for_describe_register_to___csgorequestcommandlistdescribe__;
__SCGOCommandProjectItemDescribe__ for_describe_register_to___scgocommandprojectitemdescribe__;
__SCGOCommandOperateItemDescribe__ for_describe_register_to___scgocommandoperateitemdescribe__;
__SCGORequestCommandListACKDescribe__ for_describe_register_to___scgorequestcommandlistackdescribe__;
__CSGORequestCommandDescribe__ for_describe_register_to___csgorequestcommanddescribe__;
__SCGORequestCommandAckDescribe__ for_describe_register_to___scgorequestcommandackdescribe__;
__SCGOCommandOutputDescribe__ for_describe_register_to___scgocommandoutputdescribe__;


void CSCheckServiceInfo::Serialize(ByteArray &collector) const
{
  collector.WriteInt16(service_type);
}

void CSCheckServiceInfo::Unserialize(ByteArray &collector)
{
  service_type = collector.ReadInt16();
}

void SCCheckServiceInfoAck::Serialize(ByteArray &collector) const
{
  collector.WriteInt16(service_type);
  collector.WriteString(ip_addr);
  collector.WriteUint16(port);
}

void SCCheckServiceInfoAck::Unserialize(ByteArray &collector)
{
  service_type = collector.ReadInt16();
  ip_addr = collector.ReadString();
  port = collector.ReadUint16();
}

void CSGORequestCommandList::Serialize(ByteArray &collector) const
{
  collector.WriteInt8(project_list_type);
}

void CSGORequestCommandList::Unserialize(ByteArray &collector)
{
  project_list_type = collector.ReadInt8();
}

void SCGOCommandProjectItem::Serialize(ByteArray &collector) const
{
  collector.WriteInt8(project_type);
  collector.WriteString(project_name);
}

void SCGOCommandProjectItem::Unserialize(ByteArray &collector)
{
  project_type = collector.ReadInt8();
  project_name = collector.ReadString();
}

void SCGOCommandOperateItem::Serialize(ByteArray &collector) const
{
  collector.WriteInt8(operate_type);
  collector.WriteString(operate_name);
  collector.WriteString(operate_describe);
}

void SCGOCommandOperateItem::Unserialize(ByteArray &collector)
{
  operate_type = collector.ReadInt8();
  operate_name = collector.ReadString();
  operate_describe = collector.ReadString();
}

void SCGORequestCommandListACK::Serialize(ByteArray &collector) const
{
  collector.WriteUint16((unsigned short)project_list.size());
  for (auto array_item : project_list)
  {
    array_item.Serialize(collector);
  }
  collector.WriteUint16((unsigned short)operate_list.size());
  for (auto array_item : operate_list)
  {
    array_item.Serialize(collector);
  }
}

void SCGORequestCommandListACK::Unserialize(ByteArray &collector)
{
  {
    int array_size = collector.ReadUint16();
    SCGOCommandProjectItem tmp_attr_value;
    for (int index = 0; index < array_size; ++ index)
    {
      tmp_attr_value.Unserialize(collector);
      project_list.push_back(tmp_attr_value);
    }
  }
  {
    int array_size = collector.ReadUint16();
    SCGOCommandOperateItem tmp_attr_value;
    for (int index = 0; index < array_size; ++ index)
    {
      tmp_attr_value.Unserialize(collector);
      operate_list.push_back(tmp_attr_value);
    }
  }
}

void CSGORequestCommand::Serialize(ByteArray &collector) const
{
  collector.WriteInt8(project_type);
  collector.WriteInt8(operate_type);
}

void CSGORequestCommand::Unserialize(ByteArray &collector)
{
  project_type = collector.ReadInt8();
  operate_type = collector.ReadInt8();
}

void SCGORequestCommandAck::Serialize(ByteArray &collector) const
{
  collector.WriteInt8(project_type);
  collector.WriteInt8(operate_type);
  collector.WriteInt8(operate_result);
}

void SCGORequestCommandAck::Unserialize(ByteArray &collector)
{
  project_type = collector.ReadInt8();
  operate_type = collector.ReadInt8();
  operate_result = collector.ReadInt8();
}

void SCGOCommandOutput::Serialize(ByteArray &collector) const
{
  collector.WriteUint16((unsigned short)output_str_list.size());
  for (auto array_item : output_str_list)
  {
    collector.WriteString(array_item);
  }
}

void SCGOCommandOutput::Unserialize(ByteArray &collector)
{
  {
    int array_size = collector.ReadUint16();
    std::string tmp_attr_value;
    for (int index = 0; index < array_size; ++ index)
    {
      tmp_attr_value = collector.ReadString();
      output_str_list.push_back(tmp_attr_value);
    }
  }
}

}

