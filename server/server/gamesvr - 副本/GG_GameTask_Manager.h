#ifndef GG_GAMETASK_MANAGER_H__
#define GG_GAMETASK_MANAGER_H__

#include "ggpp/GG_Task_Manager.h"
#include "GG_GameCmd.pb.h"
#include "GG_SystemCmd.pb.h"

using namespace GG_GameCmd;
using namespace GG_SystemCmd;

GG_CMD_HANDER void on_heartbeat(const GG_Message &msg);
GG_CMD_HANDER void on_game_session_close(const GG_Message &msg);
GG_CMD_HANDER void on_player_token_login(const GG_Message &msg);
GG_CMD_HANDER void on_comm_rpc(const GG_Message &msg);
GG_CMD_HANDER void on_dispatch_area_msg(const GG_Message &msg);



class GG_GameTask_Manager : public GG_Task_Manager
{
public:
	GG_GameTask_Manager(int min_thr_count = 0, int max_thr_count = 1);
	virtual ~GG_GameTask_Manager(void){};

	virtual int put_msg(const GG_Message *msg);

	CMD_REGISTER_BEGIN();
	CMD_REGISTER(GG_CMD_SESSION_CLOSE, on_game_session_close);
	CMD_REGISTER(GG_CMD_USER_TOKEN_LOGIN_GAMESVR, on_player_token_login);
	CMD_REGISTER(GG_CMD_HEARTBEAT, on_heartbeat);
	CMD_REGISTER(GG_CMD_COMM_RPC, on_comm_rpc);
	CMD_REGISTER(GG_CMD_UNKNOWN, on_dispatch_area_msg);
	CMD_REGISTER_END();

protected:
	virtual int check_msg(const GG_Message &msg);

	virtual int prep_dispath_msg(const GG_Message &msg);

	virtual void output_pool_info(void);
};


#endif  // GG_GAMETASK_MANAGER_H__

