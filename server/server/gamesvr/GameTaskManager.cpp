#include <functional>
#include "GameTaskManager.h"
#include "PlayerManager.h"
#include "RoomManager.h"
#include "libmessage/Message.h"
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

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

GameTaskManager::GameTaskManager(IoLoop& io_loop) : m_io_loop(io_loop)
{
	TRACE_FUNCATION();
	//register_cmd_handler(CMD_APPLY_ENTER_ROOM, TASK_BIND(handle_player_enter));
	register_cmd_handler(CMD_HEARTBEAT, TASK_BIND(handle_heart_beat));
	register_cmd_handler(CMD_SESSION_CLOSE, TASK_BIND(on_game_session_close));
	register_cmd_handler(CMD_USER_TOKEN_LOGIN_GAMESVR, TASK_BIND(on_player_token_login));
	register_cmd_handler(CMD_SESSION_OPEN, TASK_BIND(on_game_session_open));
}


void GameTaskManager::handle_cmd_unknown(const TcpMsgPtr& msg)
{
	RoomManagerInstance::get_mutable_instance().dispach_room_msg(msg);
}

void GameTaskManager::handle_message(const TcpMsgPtr& msg)
{
	m_io_loop.post(std::bind(&GameTaskManager::do_handle_message, this, msg));
}

void GameTaskManager::handle_heart_beat(const TcpMsgPtr& msg)
{	 
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
	PlayerManagerInstance::get_mutable_instance().flush_user_online(tcpsession_ptr);
}


void GameTaskManager::on_game_session_close(const TcpMsgPtr& msg)
{
	PlayerManagerInstance::get_mutable_instance().on_session_close(msg);
}

void GameTaskManager::on_game_session_open(const TcpMsgPtr& msg)
{
	DEBUG_LOG << "session(" << msg->session_ptr().get() << ") open";
}

void GameTaskManager::on_player_token_login(const TcpMsgPtr &msg)
{
	PlayerManagerInstance::get_mutable_instance().player_token_login(msg);
}