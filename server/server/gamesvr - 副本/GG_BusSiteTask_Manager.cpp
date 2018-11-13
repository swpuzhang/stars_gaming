#include "GG_BusSiteTask_Manager.h"
#include "GG_GamePlayer_Manager.h"
#include "GG_GameArea_Manager.h"
#include "ggpp/GG_SiteName.h"
#include "libggppcc/GG_MQService.h"
#include "GG_SvrCmd.pb.h"
#include "GG_SvrMsg.pb.h"
using namespace GG_SvrMsg;
using namespace GG_SvrCmd;

GG_BusSiteTask_Manager::GG_BusSiteTask_Manager(int min_thr_count, int max_thr_count) 
	: GG_Task_Manager(min_thr_count, max_thr_count) 
{
	set_pool_name("");
}


void on_site_session_open(const GG_Message &msg)
{
	
}

void on_get_room_alive(const GG_Message &msg)
{
    GameArea_Manager::instance()->send_flush_room_alive();
}

void on_kick_out_user(const GG_Message &msg)
{
	GamePlayer_Manager::instance()->on_kick_out_user(msg);
}

void on_hallcenter_session_open(const GG_Message &msg)
{
	GG_GameSvrStartRequest request;
	request.set_gamesvrid(GameArea_Manager::instance()->get_gamesvr_port());
	MQService::instance()->send_mqrequest(GG_CMD_GAMESVR_START, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, request);

	GG_FlushUserGameSessionRequest req;
	GamePlayer_Manager::instance()->get_ingame_user(req.mutable_onlineuserid());
	MQService::instance()->send_mqrequest(GG_CMD_FLUSH_USER_GAMESESSION, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, req);
	GamePlayer_Manager::instance()->tell_user_ingame_status(msg);
}

void on_notify_open(const GG_Message &msg)
{
	on_hallcenter_session_open(msg);
}