#pragma once

#include <set>
#include <map>
#include <string>

#include "platform/thread/mutex.hpp"

namespace face2wind
{
	enum class DebugMessageType
	{
		THREAD = 0,
		BASE_NETWORK,
		REMOTE_PROCEDURE_CALL,

		TYPE_COUNT
	};

	class DebugMessage
	{
	public:
		virtual ~DebugMessage() {}
		static DebugMessage &GetInstance();

		void SetOnshowType(DebugMessageType type, bool show_it = true);
		void ShowMessage(DebugMessageType type, const std::string &msg);

	protected:
		DebugMessage();

		std::set<DebugMessageType> onshow_type_set_;
		std::map<DebugMessageType, std::string> msg_type_to_head_str_map_;
		Mutex show_msg_lock_;
	};
}
