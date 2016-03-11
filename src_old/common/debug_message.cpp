#include "common/debug_message.hpp"
#include <iostream>

namespace face2wind
{
	DebugMessage *DebugMessage::GetInstance()
	{
		static DebugMessage instance;
		return &instance;
	}

	void DebugMessage::SetOnshowType(int type, bool show_it)
	{
		if (true == show_it)
			m_onshow_type_set.insert(type);
		else if (m_onshow_type_set.count(type) > 0)
			m_onshow_type_set.erase(type);
	}

	void DebugMessage::ShowMessage(int type, const std::string &msg)
	{
		if (m_onshow_type_set.find(type) != m_onshow_type_set.end())
		{
			std::cout << msg << std::endl;
		}
	}
	
}