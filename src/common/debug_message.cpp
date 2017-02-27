#include "common/debug_message.hpp"
#include <iostream>

namespace face2wind
{
std::set<DebugMessageType> DebugMessage::onshow_type_set_;
std::map<DebugMessageType, std::string> DebugMessage::msg_type_to_head_str_map_;
Mutex DebugMessage::show_msg_lock_;

DebugMessage::DebugMessage(DebugMessageType debug_type) : debug_type_(debug_type)
{
  this->InitMessageTypeStr();
}

DebugMessage::DebugMessage() : debug_type_(DebugMessageType::INVALID)
{
  this->InitMessageTypeStr();
}

DebugMessage &GetInstance(DebugMessageType debug_type)
{
  static DebugMessage msg_list[(int)DebugMessageType::TYPE_COUNT];
  msg_list[(int)debug_type].SetMsgType(debug_type);
  return msg_list[(int)debug_type];
}

void DebugMessage::SetOnshowType(DebugMessageType type, bool show_it)
{
  show_msg_lock_.Lock();
  
  if (true == show_it)
    onshow_type_set_.insert(type);
  else if (onshow_type_set_.count(type) > 0)
    onshow_type_set_.erase(type);

  show_msg_lock_.Unlock();
}

void DebugMessage::ShowMessage(DebugMessageType type, const std::string &msg)
{
  show_msg_lock_.Lock();

  if (onshow_type_set_.find(type) != onshow_type_set_.end())
  {
    std::cout << msg_type_to_head_str_map_[type] << msg << std::endl;
  }

  show_msg_lock_.Unlock();
}

DebugMessage &DebugMessage::operator<<(const long long &value)
{
  if (DebugMessage::onshow_type_set_.find(debug_type_) != DebugMessage::onshow_type_set_.end())
    stream_ << value;
  return *this;
}

DebugMessage &DebugMessage::operator<<(const std::string &value)
{
  if (DebugMessage::onshow_type_set_.find(debug_type_) != DebugMessage::onshow_type_set_.end())
    stream_ << value;

  return *this;
}

DebugMessage &DebugMessage::operator<<(DebugMessageCharacter c_type)
{
  if (DebugMessage::onshow_type_set_.find(debug_type_) != DebugMessage::onshow_type_set_.end())
  {
    switch (c_type)
    {
      case DebugMessageCharacter::HEAD:
        //stream_ << DebugMessage::msg_type_to_head_str_map_[debug_type_];
        break;
      case DebugMessageCharacter::ENDL:
        show_msg_lock_.Lock();
        std::cout << stream_.str() << std::endl;
        stream_.clear();
        show_msg_lock_.Unlock();
        break;
    }
  }
  return *this; 
}

void DebugMessage::InitMessageTypeStr()
{
  DebugMessage::msg_type_to_head_str_map_[DebugMessageType::THREAD] = "[thread] ";
  DebugMessage::msg_type_to_head_str_map_[DebugMessageType::BASE_NETWORK] = "[network] ";
  DebugMessage::msg_type_to_head_str_map_[DebugMessageType::REMOTE_PROCEDURE_CALL] = "[rpc] ";
}

}
