#include <functional>
#include <random>
#include <chrono>

#include "LobbyModule.h"
#include "LanchProcess.h"
#include "MqRoute.h"
#include "libtools/FunctionBindMacro.h"
#include "libtools/JsonParser.h"
#include "libtools/SystemTool.h"
#include "libresource/Mongodb.h"
#include "libresource/Redis.h"
#include "libtools/BaseTool.h"

#include "libmessage/Message.h"

void LobbyModule::dispatch_app_msg(const TcpMsgPtr &msg)
{
	std::string str_exch;
	std::string str_route;
	MsgHeader::Header &header = static_cast<MsgHeader::Header>(msg->header());
	auto iter_find = m_msg_route.begin();
	for (; iter_find != m_msg_route.end(); ++iter_find)
	{
		if (iter_find->is_cmd_match(header.cmdtype()))
		{
			break;;
		}
	}
	if (iter_find != m_msg_route.end())
	{
		str_exch = iter_find->get_exchange();
		str_exch = iter_find->get_route();
	}
	else
	{

	}
}

