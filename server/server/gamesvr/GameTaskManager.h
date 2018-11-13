#pragma once
#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"
#include <vector>
#include <unordered_map>

class GameTaskManager : public TcpTask
{
public:
	using SELF_TYPE = GameTaskManager;
	GameTaskManager(IoLoop& io_loop);
	void handle_cmd_unknown(const TcpMsgPtr& msg) override;
	void handle_message(const TcpMsgPtr&) override;
private:
	void handle_heart_beat(const TcpMsgPtr& msg);
	void on_game_session_close(const TcpMsgPtr& msg);
	void on_player_token_login(const TcpMsgPtr& msg);
	void on_game_session_open(const TcpMsgPtr& msg);
	IoLoop& m_io_loop;
};