#include "Robot.h"
#include "libtools/BaseTool.h"
#include "libtools/JsonParser.h"
#include "libserver/AsyncRequest.h"

#include "ErrorCode.pb.h"
#include "LobbyMsg.pb.h"
#include "LobbyCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"

#include <string>
#include <thread>

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

Robot::Robot(IoLoop& io_loop, IoLoop& client_io_loop, const int& user_id) : 
	TcpTask(), m_io_loop(io_loop),
	m_client_io_loop(client_io_loop),
	m_user_id(user_id)
{
	TRACE_FUNCATION();
}

void Robot::open()
{
	std::shared_ptr<TcpMsgDealer> msg_dealer = std::make_shared<TcpMsgDealer>(shared_from_this());
	const std::shared_ptr<GameMsgFactor> msg_maker = std::make_shared<GameMsgFactor>();
	m_client.reset(new Client(m_client_io_loop, msg_dealer, msg_maker));
	m_client->connect("127.0.0.1", 8000);
}

void Robot::on_unknown_msg(const TcpMsgPtr& msg)
{
	
}

void Robot::on_heart_beat(const TcpMsgPtr& msg)
{
	
}


void Robot::handle_message(const TcpMsgPtr& msg)
{
	m_io_loop.post(std::bind(&Robot::do_handle_message, shared_from_this(), msg));
}


void Robot::on_session_close(const TcpMsgPtr& msg)
{
	DEBUG_LOG << "session(" << reinterpret_cast<long>(msg->session_ptr().get()) << ") close";
}


void Robot::on_token_response(TcpMsgPtr msg, TcpMsgPtr response)
{
	if (response->header().response_result() != CODE_SUCCESS)
	{
		WARN_LOG << "send token failed:" << response->header().response_result();
		return;
	}
	const UserTokenLoginResponse& pbres = response->pbmessage<UserTokenLoginResponse>();
	INFO_LOG << "send token success";
}

void Robot::on_session_open(const TcpMsgPtr& msg)
{
	DEBUG_LOG << "session(" << reinterpret_cast<long>(msg->session_ptr().get()) << ") open";
	FULL_MSG(TcpTag::HeaderType, head, UserTokenLoginRequest, pbreq, TcpTag, sendmsg);
	head->set_cmdtype(CMD_USER_TOKEN_LOGIN);
	Json jv = {
		{"user_id", m_user_id },
		{"user_account", "zhangyang"},
		{ "user_type" , 1}
	};

	std::string strtoken = jv.dump();
	pbreq->set_usertoken(AES_ecb128_encrypt(strtoken, "key"));
	//MessagePtr<MsgTag> response;

	TcpSession::TcpAsyncFun func = std::bind(&Robot::on_token_response, shared_from_this(), PLACEHOLDER::_1, PLACEHOLDER::_2);
	if (!m_client->send_message(sendmsg, func, std::chrono::milliseconds(10000), &m_io_loop))
	{
		WARN_LOG << "send token failed";
		return;
	}
	//std::this_thread::sleep_for(std::chrono::milliseconds(20));
}