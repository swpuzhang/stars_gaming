#include "Robot.h"
#include "LanchProcess.h"

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
#include "LoginCmd.pb.h"
#include "LoginMsg.pb.h"

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
using namespace LoginMsg;
using namespace LoginCmd;

Robot::Robot(IoLoop& io_loop, IoLoop& client_io_loop, const int& user_id, const std::string& account) :
	TcpTask(), m_io_loop(io_loop),
	m_client_io_loop(client_io_loop),
	m_user_id(user_id),
	m_account(account)
{
	TRACE_FUNCATION();
	std::shared_ptr<TcpMsgDealer> msg_dealer = std::make_shared<TcpMsgDealer>(shared_from_this());
	const std::shared_ptr<GameMsgFactor> msg_maker = std::make_shared<GameMsgFactor>();
	m_client = std::make_shared <Client>(m_client_io_loop, msg_dealer, msg_maker);
	m_login_client = std::make_shared <Client>(m_client_io_loop, msg_dealer, msg_maker);
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
	DEBUG_LOG << "session(" << msg->session_ptr().get() << ")  close";
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
	if (tcpsession_ptr == m_login_client->session_ptr())
	{
		on_login_session_close(tcpsession_ptr);
	}
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
	DEBUG_LOG << "session(" << msg->session_ptr().get() << ")  open";
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
	if (tcpsession_ptr == m_login_client->session_ptr())
	{
		on_login_session_open(tcpsession_ptr);
	}
	else
	{
		
	}
	/*DEBUG_LOG << "session(" << reinterpret_cast<long>(msg->session_ptr().get()) << ") open";
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
	//std::this_thread::sleep_for(std::chrono::milliseconds(20));*/
}

void Robot::action_connect_loginsvr()
{
	auto login_host = LanchProcessInstance::get_mutable_instance().get_login_host();
	m_login_client->connect(login_host.first, login_host.second);
}

void Robot::action_token_login()
{
	UserTokenLoginRequest token_request;
	token_request.set_usertoken(m_token);
	if (!m_login_client->send_message(CMD_USER_TOKEN_LOGIN, token_request, [this](const TcpMsgPtr& request, const TcpMsgPtr& response) {
		const TcpTag::HeaderType &msg_head = response->header();
		if (msg_head.response_result() != CODE_SUCCESS)
		{
			ERROR_LOG <<  "user(" << m_user_id << ") toke_login token(" << m_token << ")";
			return;
		}
		UserTokenLoginResponse token_res = response->pbmessage<UserTokenLoginResponse>();
	}, std::chrono::milliseconds(3000), m_io_loop))
	{
		ERROR_LOG << "token login error";
		return;
	}
}

void Robot::action_login()
{
	if (!m_login_client->session_ptr())
	{
		action_connect_loginsvr();
		return;
	}

	LoginRequest login_request;
	login_request.set_account(m_account);
	
	if (!m_login_client->send_message(CMD_ROBOT_LOGIN, login_request, [this](const TcpMsgPtr& request, const TcpMsgPtr& response) {
		const TcpTag::HeaderType &msg_head = response->header();
		if (msg_head.response_result() != CODE_SUCCESS)
		{
			ERROR_LOG << "robot account(" << m_account;
			return;

		}
		LoginResponse login_res = response->pbmessage<LoginResponse>();
		m_user_id = login_res.user_id();
		m_lobby_ip = login_res.lobby_ip();
		m_lobby_port = login_res.lobby_port();
		m_token = login_res.token();
		m_is_need_login = false;
		m_last_login_time = steady_clock::now();
		m_login_client->close_client();
		if (!m_client->session_ptr())
		{
			m_client->connect(m_lobby_ip, m_lobby_port);
		}
		}, std::chrono::milliseconds(3000), m_io_loop))
	{
		action_connect_loginsvr();
		ERROR_LOG << "send loginsvr error";
		return;
	}
}

void Robot::on_login_session_open(const TcpSessionPtr& session)
{
	action_login();
}

void Robot::on_login_session_close(const TcpSessionPtr& session)
{

}

void Robot::on_lobby_session_open(const TcpSessionPtr& session)
{

}

void Robot::on_lobby_session_close(const TcpSessionPtr& session)
{

}