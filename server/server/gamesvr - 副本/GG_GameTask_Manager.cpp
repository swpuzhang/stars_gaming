#include "GG_GameTask_Manager.h"
#include "logger/ilog.h"
#include "GG_GamePlayer_Manager.h"
#include "GG_GameArea_Manager.h"
#include "GG_GameMsg.pb.h"

GG_GameTask_Manager::GG_GameTask_Manager(int min_thr_count, int max_thr_count) 
	: GG_Task_Manager(min_thr_count, max_thr_count) 
{
	set_pool_name("game-task");
}

int GG_GameTask_Manager::check_msg(const GG_Message &msg)
{
	TRACE_FUNCATION();
	if (-1 == GG_Task_Manager::check_msg(msg))
	{
		LOG_ERROR("check msg failed");
		return -1;
	}

	return 0;
}

int GG_GameTask_Manager::prep_dispath_msg(const GG_Message &msg)
{
	return 0;
}

int GG_GameTask_Manager::put_msg(const GG_Message *msg)
{
	TRACE_FUNCATION();
	dispatch_msg(*msg);
	LOG_TRACE("del msg:" + toString(msg));
	delete msg;
	return 0;
}


void on_heartbeat(const GG_Message &msg)
{
    TRACE_FUNCATION();
	///TODO：这里是否要换成GameArea里的刷新
    GamePlayer_Manager::instance()->flush_user_online(msg.session_id());
}

void on_game_session_close(const GG_Message &msg)
{
	GamePlayer_Manager::instance()->on_session_close(msg);
}

void on_player_token_login(const GG_Message &msg)
{
	GamePlayer_Manager::instance()->on_player_token_login(msg);
}

void on_comm_rpc(const GG_Message &msg)
{

}

void on_dispatch_area_msg(const GG_Message &msg)
{
	GameArea_Manager::instance()->dispatch_area_msg(msg);
}


void GG_GameTask_Manager::output_pool_info(void)
{
	LOG_INFO("ps-gametask(" + toString(this) + "|" + get_pool_name() + ") thr_count(cur:" + toString(this->thr_count()) + "|min:" + toString(m_min_thr_count) + "|max:" + toString(m_max_thr_count) + ") queue(" + toString(this->msg_queue()->message_count()) + ")");
	GameArea_Manager::instance()->output_manager_info();
	GamePlayer_Manager::instance()->output_manager_info();
}

