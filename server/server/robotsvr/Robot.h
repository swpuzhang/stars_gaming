#ifndef ROBOT_H
#define ROBOT_H

#include "GameMsgFactor.h"
#include "libserver/Client.h"
#include "libserver/TcpSession.h"
#include "libtools/AsioTypes.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/Message.h"
#include "libmessage/MessageMaker.h"
#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"
#include "libtools/IoLoopPool.h"
#include "liblog/Log.h"
#include <memory>
#include <chrono>

using namespace std::chrono;

class Robot : public std::enable_shared_from_this<Robot>, public TcpTask
{
public:

	using SELF_TYPE = Robot;
	Robot(IoLoop& io_loop, IoLoop& client_io_loop, const int& user_id, const std::string& account);
	void handle_message(const TcpMsgPtr& msg) override;
	void set_account(const std::string& account) { m_account = account; }
	void set_version(const std::string& account) { m_account = account; }
protected:
	void on_unknown_msg(const TcpMsgPtr& msg) override;
	void on_session_close(const TcpMsgPtr& msg) override;
	void on_session_open(const TcpMsgPtr& msg) override;
	void on_heart_beat(const TcpMsgPtr& msg) override;
	//void on_token_login(const TcpMsgPtr& msg);
private:
	void action_connect_loginsvr();
	void action_login();
	void action_token_login();
	void on_login_session_open(const TcpSessionPtr& session);
	void on_login_session_close(const TcpSessionPtr& session);
	void on_lobby_session_open(const TcpSessionPtr& session);
	void on_lobby_session_close(const TcpSessionPtr& session);
private:
	int m_user_id = 0;
	std::string m_account;
	std::string m_lobby_ip;
	TY_UINT16 m_lobby_port;
	std::string m_token;
	bool m_is_need_login = false;
	const static int MAX_LOGIN_INTERVAL = 3600 * 24;
	time_point<steady_clock> m_last_login_time = steady_clock::now();
	IoLoop& m_io_loop;
	IoLoop& m_client_io_loop;
	std::unique_ptr<Client> m_client;
	std::unique_ptr<Client> m_login_client;
};
#endif