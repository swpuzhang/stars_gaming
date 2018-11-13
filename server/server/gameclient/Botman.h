#pragma once
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


class Botman : public std::enable_shared_from_this<Botman>, public TcpTask
{
public:

	using SELF_TYPE = Botman;
	Botman(IoLoop& io_loop, IoLoop& client_io_loop, int user_id);
	void open();

	void handle_cmd_unknown(const TcpMsgPtr& msg) override;
	void handle_message(const TcpMsgPtr& msg) override;
private:
	void handle_heart_beat(const TcpMsgPtr& msg);
	void on_game_session_close(const TcpMsgPtr& msg);
	void on_game_session_open(const TcpMsgPtr& msg);
	void on_token_response(TcpMsgPtr msg, TcpMsgPtr response);
private:
	int m_user_id;
	IoLoop& m_io_loop;
	IoLoop& m_client_io_loop;
	std::unique_ptr<Client> m_client;
};
