#ifndef GG_SECURITY_TASK_MANAGER_H__
#define GG_SECURITY_TASK_MANAGER_H__

#include "ggpp/GG_Task_Manager.h"
//#include "GG_DCenter_Command.h"
//#include "GG_SvrInteract_Message.h"


GG_CMD_HANDER void on_securiy_svr_session_open(const GG_Message &msg);
//GG_CMD_HANDER void handle_rpc_call(const GG_Message &msg);
GG_CMD_HANDER void on_kick_out_user(const GG_Message &msg);


class GG_SecurityTask_Manager : public GG_Task_Manager
{
public:
	GG_SecurityTask_Manager(int min_thr_count = 1, int max_thr_count = 1);
	virtual ~GG_SecurityTask_Manager(void){};

	/*CMD_REGISTER_BEGIN();
	CMD_REGISTER(GG_CMD_SESSION_OPEN, on_securiy_svr_session_open);
	//CMD_REGISTER(GG_CMD_COMM_RPC, handle_rpc_call);
	CMD_REGISTER(GG_CMD_KICK_OUT_USER, on_kick_out_user);
	CMD_REGISTER_END();*/

};


#endif //GG_SECURITY_TASK_MANAGER_H__
