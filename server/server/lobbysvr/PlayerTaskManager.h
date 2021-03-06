#ifndef LOGIN_TASK_MANAGER_H
#define LOGIN_TASK_MANAGER_H

#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"
#include <vector>
#include <unordered_map>

class LobbyModule;

class PlayerTaskManager : public TcpTask
{
public:
	using SELF_TYPE = PlayerTaskManager;
	PlayerTaskManager(IoLoop& io_loop);
	void handle_message(const TcpMsgPtr&) override;

protected:
	void on_unknown_msg(const TcpMsgPtr& msg) override;
	void on_session_close(const TcpMsgPtr& msg) override;
	void on_session_open(const TcpMsgPtr& msg) override;
	void on_heart_beat(const TcpMsgPtr& msg) override;
	void on_token_login(const TcpMsgPtr& msg);
private:
	
	IoLoop& m_io_loop;
};

#endif // !LOGIN_TASK_MANAGER_H