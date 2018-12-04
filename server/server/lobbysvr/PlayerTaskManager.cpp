#include <functional>
#include <boost/optional.hpp>
#include "PlayerTaskManager.h"
#include "PlayerManager.h"
#include "LobbyModule.h"
#include "libmessage/Message.h"
#include "libserver/TcpSession.h"

#include "ErrorCode.pb.h"
#include "LobbyMsg.pb.h"
#include "LobbyCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"

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

PlayerTaskManager::PlayerTaskManager(IoLoop& io_loop) : m_io_loop(io_loop)
{
	TRACE_FUNCATION();
	register_msg_handler(LobbyCmd::CMD_USER_TOKEN_LOGIN, TASK_BIND(on_token_login));
}


void PlayerTaskManager::handle_message(const TcpMsgPtr& msg)
{
	m_io_loop.post(std::bind(&PlayerTaskManager::do_handle_message, this, msg));
}

void PlayerTaskManager::on_heart_beat(const TcpMsgPtr& msg)
{	 
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
	PlayerManagerInstance::get_mutable_instance().flush_user_online(tcpsession_ptr);
}


void PlayerTaskManager::on_session_close(const TcpMsgPtr& msg)
{
	DEBUG_LOG << "session(" << msg->session_ptr().get() << ")  close";
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
	PlayerManagerInstance::get_mutable_instance().on_session_close(msg);
}

void PlayerTaskManager::on_session_open(const TcpMsgPtr& msg)
{
	DEBUG_LOG << "session(" << msg->session_ptr().get() << ")  open";
}

void PlayerTaskManager::on_token_login(const TcpMsgPtr& msg)
{
	PlayerManagerInstance::get_mutable_instance().player_token_login(msg);
}

void PlayerTaskManager::on_unknown_msg(const TcpMsgPtr& msg)
{
	boost::optional<int> user_id = PlayerManagerInstance::get_mutable_instance().check_user_msg(msg);
	if (!user_id)
	{
		msg->send_failed_reason(CODE_SESSION_HAV_NO_USER);
		return;
	}
	LobbyInstance::get_mutable_instance().dispatch_app_msg(user_id, msg);
}