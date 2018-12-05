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
	Robot(IoLoop& io_loop, IoLoop& client_io_loop, const int& user_id);
	void open();
	void handle_message(const TcpMsgPtr& msg) override;

protected:
	void on_unknown_msg(const TcpMsgPtr& msg) override;
	void on_session_close(const TcpMsgPtr& msg) override;
	void on_session_open(const TcpMsgPtr& msg) override;
	void on_heart_beat(const TcpMsgPtr& msg) override;
	//void on_token_login(const TcpMsgPtr& msg);

private:
	int m_user_id = 0;
	bool m_is_need_login = false;
	const static int MAX_LOGIN_INTERVAL = 3600 * 24;
	time_point<steady_clock> m_last_login_time = steady_clock::now();
	IoLoop& m_io_loop;
	IoLoop& m_client_io_loop;
	std::unique_ptr<Client> m_client;
	std::unique_ptr<Client> m_login_client;
};
#endif