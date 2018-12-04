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
#include "LobbyCmd.pb.h"
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
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;
using namespace LoginCmd;
using namespace LoginMsg;

using namespace std::chrono;


class NormalRout
{
public:
	NormalRout(const std::string& exch, const std::string& route);
	std::pair<std::string, std::string> operator() (int user_id, const TcpMsgPtr& msg) const;
private:
	std::string m_exchange;
	std::string m_route;
};

class HashRout
{
public:
	HashRout(const Json& hash_json, const std::string& exch, const std::string& route);

	std::pair<std::string, std::string> operator() (int user_id, const TcpMsgPtr& msg) const;

private:
	int m_total_cuont = 1;
	std::string m_exchange;
	std::string m_route;
};

class RoomRout
{
public:
	RoomRout(const Json& room_json, const std::string& exch, const std::string& route);
	std::pair<std::string, std::string> operator() (int user_id, const TcpMsgPtr& msg) const;
private:
	int m_room_id_begin = 100;
	int m_room_count_per_svr = 100;
	std::string m_exchange;
	std::string m_route;
};

class MsgRout
{
public:
	enum
	{
		NORMAL_ROUTE,
		HASH_ROUTE,
		ROOM_ROUTE,
	};
	MsgRout(int cmd_begin, int cmd_end, const std::string exch, const std::string& route, int route_type, const Json& special_json);
	bool is_cmd_match(const int msg_cmd) const;
	std::pair<std::string, std::string> get_ex_route(int user_id, const TcpMsgPtr& msg) const;
private:
	int m_cmd_begin =0 ;
	int m_cmd_end = 0;
	std::function<std::pair<std::string, std::string>(int user_id, const TcpMsgPtr)> m_route_fun;
};

class RoomRout
{

};

class LobbyModule
{
public:
	LobbyModule() = default;
	~LobbyModule() = default;
	void open(const std::shared_ptr<IoLoop>& io_loop) { m_io_loop = io_loop; }
	void parse_route_config(const Json& route_json);
	using SELF_TYPE = LobbyModule;
	void dispatch_app_msg(int user_id, const TcpMsgPtr &msg);
public:
	
private:
	std::shared_ptr<IoLoop> m_io_loop;
	std::vector<MsgRout> m_msg_route;
};

using LobbyInstance = boost::serialization::singleton<LobbyModule>;

#endif // !LOGIN_TASK_MANAGER_H