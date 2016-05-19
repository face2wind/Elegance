#include "common/debug_message.hpp"
#include <iostream>

namespace face2wind
{
	DebugMessage::DebugMessage()
	{
		msg_type_to_head_str_map_[DebugMessageType::THREAD] = "[thread] ";
		msg_type_to_head_str_map_[DebugMessageType::BASE_NETWORK] = "[network] ";
		msg_type_to_head_str_map_[DebugMessageType::REMOTE_PROCEDURE_CALL] = "[rpc] ";
	}

	DebugMessage &DebugMessage::GetInstance()
	{
		static DebugMessage instance;
		return instance;
	}

	void DebugMessage::SetOnshowType(DebugMessageType type, bool show_it)
	{
		if (true == show_it)
			onshow_type_set_.insert(type);
		else if (onshow_type_set_.count(type) > 0)
			onshow_type_set_.erase(type);
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
	
}