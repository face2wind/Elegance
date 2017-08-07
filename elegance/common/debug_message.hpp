#pragma once

#include <set>
#include <map>
#include <string>
#include <sstream>

#include "elegance/platform/thread/mutex.hpp"

namespace face2wind
{
enum class DebugMessageType
{
  INVALID = 0,
  THREAD,
  BASE_NETWORK,
  REMOTE_PROCEDURE_CALL,

  TYPE_COUNT
};

enum class DebugMessageCharacter
{
  HEAD = 0,
  ENDL,
};

class DebugMessage
{
 public:
  DebugMessage(DebugMessageType debug_type);
  DebugMessage();
  virtual ~DebugMessage() {}

  static DebugMessage &GetInstance(DebugMessageType debug_type);
  
  static void SetOnshowType(DebugMessageType type, bool show_it = true);
  static void ShowMessage(DebugMessageType type, const std::string &msg);

  void SetMsgType(DebugMessageType debug_type) { debug_type_ = debug_type;  }

  DebugMessage &operator<<(const long long &value);
  DebugMessage &operator<<(const std::string &value);
  DebugMessage &operator<<(DebugMessageCharacter c_type);
  
 protected:
  void InitMessageTypeStr();
  
  DebugMessageType debug_type_;
  std::stringstream stream_;
  
  static std::set<DebugMessageType> onshow_type_set_;
  static std::map<DebugMessageType, std::string> msg_type_to_head_str_map_;
  static Mutex show_msg_lock_;
};

#define fDebug(debug_type) DebugMessage(debug_type)
//#define fDebugInstance(debug_type) DebugMessage::GetInstance(debug_type)  instance will be not safe with multi thread
#define fDebugWithHead(debug_type) fDebug(debug_type)<<DebugMessageCharacter::HEAD
#define fDebugEndl DebugMessageCharacter::ENDL

}
