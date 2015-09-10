#ifndef _DEBUG_MESSAGE_HPP_
#define _DEBUG_MESSAGE_HPP_

#include <set>
#include <string>

namespace face2wind
{
	enum DebugMessageType
	{
		DEBUG_MESSAGE_TYPE_BASE_NETWORK = 0,
	};

	class DebugMessage
	{
	public:
		virtual ~DebugMessage() {}
		static DebugMessage *GetInstance();

		void SetOnshowType(int type, bool show_it = true);
		void ShowMessage(int type, const std::string &msg);

	protected:
		DebugMessage() {}

		std::set<int> m_onshow_type_set;
	};
}

#endif