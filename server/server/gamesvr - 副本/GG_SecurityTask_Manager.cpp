#include "GG_SecurityTask_Manager.h"
#include "GG_GamePlayer_Manager.h"

GG_SecurityTask_Manager::GG_SecurityTask_Manager(int min_thr_count, int max_thr_count) 
	: GG_Task_Manager(min_thr_count, max_thr_count) 
{
	set_pool_name("security-task");
}


void on_securiy_svr_session_open(const GG_Message &msg)
{
	
}

void on_kick_out_user(const GG_Message &msg)
{
	GamePlayer_Manager::instance()->on_kick_out_user(msg);
}

