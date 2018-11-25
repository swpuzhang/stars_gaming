#include "RoomManager.h"
#include "GameRoom.h"
#include "liblog/Log.h"
#include "ErrorCode.pb.h"
#include "LobbyMsg.pb.h"
#include "LobbyCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

void RoomManager::open(TY_UINT16 svr_port, IoLoop* io_loop)
{
	m_svr_port = svr_port;
	m_io_loop = io_loop;
	auto roomids = get_room_id();
	for (auto &e : roomids)
	{
		add_room(e);
	}
}

bool RoomManager::check(int user_id, int room_id) const
{
	if (m_game_room.find(room_id) == m_game_room.end())
	{
		return false;
	}
	return true;
}

std::vector<int> RoomManager::get_room_id()
{
	std::vector<int> roomids = { 1, 2, 3, 4, 5 };
	return roomids;
}

void RoomManager::add_room(int room_id)
{
	GAMEROOM_PTR one_room = std::make_shared<GameRoom>(room_id);
	m_game_room.insert({ room_id, one_room });
	one_room->open();
}

void RoomManager::dispach_room_msg(const TcpMsgPtr& msg) const
{
	/*const UserRoomMsg& gamemsg = msg->message<UserRoomMsg>();
	auto itroom = m_game_room.find(gamemsg.roomid());
	if (itroom == m_game_room.end())
	{
		msg->send_failed_reason(CODE_INVALID_ROOMID);
		ERROR_LOG << "check msg(" << msg->header().cmdtype() << ")failed";
		return;
	}
	itroom->second->handle_msg(msg);*/
}