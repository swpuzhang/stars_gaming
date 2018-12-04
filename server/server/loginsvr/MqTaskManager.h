#ifndef MQ_TASK_MANAGER_H
#define MQ_TASK_MANAGER_H

#include <vector>
#include <unordered_map>

#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"


class MqTaskManager : public MqTask
{
public:
	using SELF_TYPE = MqTaskManager;
	MqTaskManager(IoLoop& io_loop);
	void handle_message(const MqMsgPtr&) override;

protected:
	void on_session_close(const MqMsgPtr& msg) override;
	void on_session_open(const MqMsgPtr& msg) override;
private:
	
	IoLoop& m_io_loop;
};

#endif // !LOGIN_TASK_MANAGER_H