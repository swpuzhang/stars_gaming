#ifndef GG_BUSSITE_TASK_MANAGER_H__
#define GG_BUSSITE_TASK_MANAGER_H__

#include "ggpp/GG_Task_Manager.h"

#include "GG_SystemCmd.pb.h"
#include "GG_RoomCmd.pb.h"
#include "GG_SvrCmd.pb.h"

GG_CMD_HANDER void on_site_session_open(const GG_Message &msg);
GG_CMD_HANDER void on_get_room_alive(const GG_Message &msg);
GG_CMD_HANDER void on_hallcenter_session_open(const GG_Message &msg);
GG_CMD_HANDER void on_notify_open(const GG_Message &msg);
GG_CMD_HANDER void on_kick_out_user(const GG_Message &msg);

class GG_BusSiteTask_Manager : public GG_Task_Manager
{
public:
	GG_BusSiteTask_Manager(int min_thr_count = 1, int max_thr_count = 1);
	virtual ~GG_BusSiteTask_Manager(void){};

	CMD_REGISTER_BEGIN();
	CMD_REGISTER(GG_SystemCmd::GG_CMD_SESSION_OPEN, on_site_session_open);
    CMD_REGISTER(GG_RoomCmd::GG_CMD_GET_ROOM_ALIVE, on_get_room_alive);
	CMD_REGISTER(GG_SystemCmd::GG_CMD_SESSION_OPEN, on_hallcenter_session_open);
	CMD_REGISTER(GG_SvrCmd::GG_CMD_KICK_OUT_USER, on_kick_out_user);
	CMD_REGISTER(GG_SvrCmd::GG_CMD_NOTIFY_OPEN, on_notify_open);
	CMD_REGISTER_END();

};


#endif //GG_BUSSITE_TASK_MANAGER_H__
