#ifndef LOGIN_MODULE_MANAGER_H
#define LOGIN_MODULE_MANAGER_H

#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"
#include "libtools/Types.h"
#include "libtools/AsioTypes.h"
#include "libresource/Redis.h"
#include "libtools/JsonParser.h"
#include "UserCommon.h"
#include <boost/serialization/singleton.hpp>
#include <vector>
#include <map>
#include <chrono>

#include "ErrorCode.pb.h"
#include "LobbyMsg.pb.h"
#include "HallCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"
#include "libserver/TcpSession.h"
#include "LoginMsg.pb.h"
#include "LoginCmd.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace HallCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;
using namespace LoginCmd;
using namespace LoginMsg;

using namespace std::chrono;

class MsgRout
{
public:
	void set_route(int cmd_begin, int cmd_end, const std::string& exchange, const std::string& route)
	{
		m_cmd_begin = cmd_begin;
		m_cmd_end = cmd_end;
		m_exchange = exchange;
		m_route = route;
	}
	bool is_cmd_match(const int msg_cmd) const
	{
		if (msg_cmd >= m_cmd_begin && msg_cmd <= m_cmd_end)
		{
			return true;
		}
		return false;
	}
	const std::string& get_exchange() const {
		return  m_exchange;
	}
	const std::string& get_route() const {
		return  m_route;
	}
private:
	int m_cmd_begin =0 ;
	int m_cmd_end = 0;
	std::string m_exchange;
	std::string m_route;
};

class LobbyModule
{
public:
	using SELF_TYPE = LobbyModule;
	LobbyModule() {}
	void dispatch_app_msg(const TcpMsgPtr &msg);
private:


public:
	
private:
	std::vector<MsgRout> m_msg_route;
};

using LobbyInstance = boost::serialization::singleton<LobbyModule>;

#endif // !LOGIN_TASK_MANAGER_H