#include <functional>
#include "LoginTaskManager.h"
#include "libmessage/Message.h"
#include "ErrorCode.pb.h"
#include "HallMsg.pb.h"
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
using namespace HallMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;
using namespace LoginCmd;
using namespace LoginMsg;

LoginTaskManager::LoginTaskManager(IoLoop& io_loop) : m_io_loop(io_loop)
{
	TRACE_FUNCATION();
	register_msg_handler(CMD_LOGIN, TASK_BIND(on_login));
}


void LoginTaskManager::handle_message(const TcpMsgPtr& msg)
{
	m_io_loop.post(std::bind(&LoginTaskManager::do_handle_message, this, msg));
}

void LoginTaskManager::on_heart_beat(const TcpMsgPtr& msg)
{	 
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
}


void LoginTaskManager::on_session_close(const TcpMsgPtr& msg)
{
	DEBUG_LOG << "session(" << msg->session_ptr().get() << ")  close";
	LoginInstance::get_mutable_instance().user_session_close(msg->session_ptr());
}

void LoginTaskManager::on_session_open(const TcpMsgPtr& msg)
{
	DEBUG_LOG << "session(" << msg->session_ptr().get() << ")  open";
	LoginInstance::get_mutable_instance().user_session_open(msg->session_ptr());
}

void LoginTaskManager::on_login(const TcpMsgPtr& msg)
{
	LoginInstance::get_mutable_instance().user_login(msg);
}