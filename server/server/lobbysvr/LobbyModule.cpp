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


MsgRout::MsgRout(int cmd_begin, int cmd_end, const std::string exch, const std::string& route,
	int route_type, const Json& special_json) :
	m_cmd_begin(cmd_begin),
	m_cmd_end(cmd_end)
{
	switch (route_type)
	{
	case NORMAL_ROUTE:
		m_route_fun = NormalRout(exch, route);
		break;
	case HASH_ROUTE:
		m_route_fun = HashRout(special_json, exch, route);
		break;
	case ROOM_ROUTE:
		m_route_fun = RoomRout(special_json, exch, route);
		break;
	default:
		ERROR_LOG << "route_type error:" << route_type;
		break;
	}
}
bool MsgRout::is_cmd_match(const int msg_cmd) const
{
	if (msg_cmd >= m_cmd_begin && msg_cmd <= m_cmd_end)
	{
		return true;
	}
	return false;
}
std::pair<std::string, std::string> MsgRout::get_ex_route(int user_id, const TcpMsgPtr& msg) const
{
	return m_route_fun(user_id, msg);
}

RoomRout::RoomRout(const Json& room_json, const std::string& exch, const std::string& route) :
	m_room_id_begin(atoi( room_json["room_begin"].get<std::string>().c_str())),
	m_room_count_per_svr(atoi(room_json["count_per_svr"].get<std::string>().c_str())),
	m_exchange(exch),
	m_route(route)
{
}

std::pair<std::string, std::string> RoomRout::operator() (int user_id, const TcpMsgPtr& msg) const
{
	int room_id = 1000;
	int route_begin = room_id / m_room_count_per_svr;
	std::string route_key = m_route + "-" + std::to_string(route_begin);
	return{ m_exchange,  std::move(route_key) };
}

HashRout::HashRout(const Json& hash_json, const std::string& exch, const std::string& route) :
	m_total_cuont(atoi(room_json["total_count"].get<std::string>().c_str())),
	m_exchange(exch),
	m_route(route)
{
}

std::pair<std::string, std::string> HashRout::operator() (int user_id, const TcpMsgPtr& msg) const
{
	int route_begin = user_id % m_total_cuont;
	std::string route_key = m_route + "-" + std::to_string(route_begin);
	return{ m_exchange,  std::move(route_key) };
}

NormalRout::NormalRout(const std::string& exch, const std::string& route) :
	m_exchange(exch),
	m_route(route)
{
}
std::pair<std::string, std::string> NormalRout::operator() (int user_id, const TcpMsgPtr& msg) const
{
	return{ m_exchange , m_route };
}

void LobbyModule::dispatch_app_msg(int user_id, const TcpMsgPtr &msg)
{
	std::string str_exch;
	std::string str_route;
	MsgHeader::Header &header = static_cast<MsgHeader::Header>(msg->header());
	auto iter_find = m_msg_route.begin();
	for (; iter_find != m_msg_route.end(); ++iter_find)
	{
		if (iter_find->is_cmd_match(header.cmdtype()))
		{
			break;
		}
	}
	if (iter_find != m_msg_route.end())
	{
		auto ex_route = iter_find->get_ex_route(user_id, msg);
		str_exch = ex_route.first;
		str_exch = ex_route.second;
	}
	else
	{
		ERROR_LOG << "error cmd:" << header.cmdtype();
		msg->send_failed_reason(CODE_PARAMETER_INVALID);
		return;
	}

	if (!LanchProcessInstance::get_mutable_instance().get_mq().send_message(header.cmdtype(),
		msg->pbmessage_ptr(), str_exch, str_route,
		[msg](const MqMsgPtr& request, const MqMsgPtr& response)
	{
		msg->send_response(response);
	},
		std::chrono::milliseconds(3000), m_io_loop.get()))
	{
		msg->send_failed_reason(CODE_SYSTEM_ERROR);
	}
}

void LobbyModule::parse_route_config(const Json& route_json)
{
	for (auto &jv_one : route_json)
	{
		m_msg_route.emplace_back(MsgRout(jv_one["cmd_begin"].get<int>(), 
			atoi(jv_one["cmd_end"].get<std::string>().c_str()), 
			jv_one["exchange"].get<std::string>(), 
			jv_one["route"].get<std::string>(), 
			atoi(jv_one["type"].get<std::string>().c_str()),
			jv_one["special"]));
	}
}

