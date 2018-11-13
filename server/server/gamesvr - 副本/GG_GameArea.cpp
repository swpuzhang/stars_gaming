#include "GG_GameArea.h"
#include "logger/ilog.h"
#include "ace/Reactor.h"
#include "GG_GameDesk.h"
#include "GG_Player.h"
#include "GG_GamePlayer_Manager.h"
#include "GG_Card_Dealer.h"
#include "GG_GameArea_Manager.h"
#include <assert.h>
#include <algorithm>
#include "GG_Game_Process.h"
#include "libggppcc/GG_SiteService.h"
#include "GG_Money_Proxy.h"
#include "libggppcc/GG_MQService.h"
#include "GG_Money_Proxy.h"


#define GG_SQL_GAMESVR_LOAD_ROOM_INFO "GG_SQL_GAMESVR_LOAD_ROOM_INFO"

const int ONE_DEAL_GAME_DESK = 0;
const int GAME_DESK_QUICK = 1;

#define REGISTER_RPC_METHOD(rpc_name) this->register_rpc_handler(#rpc_name, &GG_GameArea::rpc_name)


Json::Value parse_json_string(const std::string& json_string)
{
	Json::Value root;
	Json::Reader reader(Json::Features::strictMode());

	if (!reader.parse(json_string, root))
	{
		root = Json::Value();
		//LOG_ERROR("parse room list failed");
	}
	return root;
}

std::string json_to_string(const Json::Value& json_value)
{
	Json::FastWriter writer;  
	return writer.write(json_value);
}


void GG_GameArea::GameWaitInterval::parse(const GG_DBRow& db_row)
{
	int col = 11;
	int deal_card = atoi(db_row.dbrow(col++).c_str());
	m_time_deal_card.set(deal_card / 1000, (deal_card % 1000) * 1000);

	int time_active1 = atoi(db_row.dbrow(col++).c_str());
	m_time_active1.set(time_active1 / 1000, (time_active1 % 1000) * 1000); // 2s 发牌动画时间

	int time_active2 = atoi(db_row.dbrow(col++).c_str());
	m_time_active2.set(time_active2 / 1000, (time_active2 % 1000) * 1000);

	int time_shoot = atoi(db_row.dbrow(col++).c_str());
	m_time_post_card.set(time_shoot / 1000, (time_shoot % 1000) * 1000);

	int time_shuffle = atoi(db_row.dbrow(col++).c_str());
	m_time_shuffle_card.set(time_shuffle / 1000, (time_shuffle % 1000) * 1000);
	
	int time_drop = atoi(db_row.dbrow(col++).c_str());
	m_time_drop.set(time_drop / 1000, (time_drop % 1000) * 1000);

	int time_account_animat = atoi(db_row.dbrow(col++).c_str());
	m_time_account_animat.set(time_account_animat / 1000, (time_account_animat % 1000) * 1000);
	

	int time_collect_chips = atoi(db_row.dbrow(col++).c_str());
	m_time_distribute.set(time_collect_chips / 1000, (time_collect_chips % 1000) * 1000);

	int time_has_shoot = atoi(db_row.dbrow(col++).c_str());
	m_time_declare.set(time_has_shoot / 1000, (time_has_shoot % 1000) * 1000);

	int time_dragon = atoi(db_row.dbrow(col++).c_str());
	m_time_wait_next_game.set(time_dragon / 1000, (time_dragon % 1000) * 1000);

	m_time_declare_animat.set(3, 0);
}


void GG_GameArea::AreaInfo::parse(const GG_DBRow& db_row)
{
	int col = 0;
	m_area_id = atoi(db_row.dbrow(col++).c_str());
	m_small_blind = BaseTool::atoi64(db_row.dbrow(col++));
	m_min_carry = BaseTool::atoi64(db_row.dbrow(col++));
	m_max_carry = BaseTool::atoi64(db_row.dbrow(col++));
	m_max_playing = atoi(db_row.dbrow(col++).c_str());
	m_room_type = atoi(db_row.dbrow(col++).c_str());
	m_base_exp_win = atoi(db_row.dbrow(col++).c_str());
	m_base_exp_lose = atoi(db_row.dbrow(col++).c_str());
	m_tips_persent = atoi(db_row.dbrow(col++).c_str());
	m_desk_type = atoi(db_row.dbrow(col++).c_str());
	m_draw_tips = BaseTool::atoi64(db_row.dbrow(col++));
	m_game_wait_interval.parse(db_row);
	
}

//////////////////////////////////////////////////////////////////////////


int GG_GameArea::check_online_callback(const GG_Message &request, void *args, const GG_Response &response)
{
	GG_GameArea *game_area = (GG_GameArea*)args;
	if (!response.success())
	{
		return 0;
	}

	game_area->on_recv_msg(response);
	return 0;
}

GG_GameArea::GG_GameArea(int id)
	:m_area_id(id), m_game_desk(NULL), m_pCardDealer(NULL)
{
	register_msg_handler(GG_CMD_APPLY_ENTER_ROOM, &GG_GameArea::on_player_enter);
	register_msg_handler(GG_CMD_APPLY_SEAT, &GG_GameArea::on_apply_seat);
	register_msg_handler(GG_CMD_APPLY_QUIT_ROOM, &GG_GameArea::on_player_exit);

	register_msg_handler(GG_CMD_APPLY_STANDUP, &GG_GameArea::on_apply_standup);
	register_msg_handler(GG_CMD_APPLY_FLUSHROOM, &GG_GameArea::on_apply_flush_room);
	register_msg_handler(GG_CMD_APPLY_FACESHOW, &GG_GameArea::on_apply_face_show);
	register_msg_handler(GG_CMD_APPLY_CHAT, &GG_GameArea::on_apply_chat);
	register_msg_handler(GG_CMD_APPLY_USE_PROPS, &GG_GameArea::on_apply_use_props);
	register_msg_handler(GG_CMD_APPLY_PRESENT_CHIPS, &GG_GameArea::on_apply_present_chips);
	register_msg_handler(GG_CMD_COMM_RPC, &GG_GameArea::on_call_comm_rpc);
	register_msg_handler(GG_CMD_APPLY_PRESENT_GIFT, &GG_GameArea::on_apply_present_gift);
	register_msg_handler(GG_CMD_APPLY_TRANSMIT_MESSAGE, &GG_GameArea::on_apply_transmit_message);
	register_msg_handler(GG_CMD_APPLY_PRESENT_GIFT_NEWINTER, &GG_GameArea::on_apply_present_gift_newinter);
	register_msg_handler(GG_CMD_APPLY_POST_CARD, &GG_GameArea::on_apply_post_card);
	register_msg_handler(GG_CMD_APPLY_DIG_CARD, &GG_GameArea::on_apply_dig_card);
	register_msg_handler(GG_CMD_APPLY_DROP, &GG_GameArea::on_apply_drop);
	register_msg_handler(GG_CMD_APPLY_DECLARE, &GG_GameArea::on_apply_declare);
	register_msg_handler(GG_CMD_APPLY_LOSE_DECLARE, &GG_GameArea::on_apply_lose_declare);
	register_msg_handler(GG_CMD_APPLY_ROOM_STATUS, &GG_GameArea::on_apply_room_status);
	register_msg_handler(GG_CMD_APPLY_TRUSTEE_OFF, &GG_GameArea::on_apply_trustee_off);		// 申请取消托管
	register_msg_handler(GG_CMD_APPLY_CLOSE_ACCOUNT_BOX, &GG_GameArea::on_apply_close_account_box);
}


GG_GameArea::~GG_GameArea(void)
{
	delete m_game_desk;
	delete m_pCardDealer;
}

int GG_GameArea::open(void *args)
{
	TRACE_FUNCATION();
	if (-1 == load_area_info())
	{
		LOG_ERROR("load area info failed");
		return -1;
	}

	m_pCardDealer = make_card_dealer();
	m_game_desk = make_game_desk();
	m_game_desk->reactor(reactor());
	m_pCardDealer->init_rand_seed();

	return 0;
}

int GG_GameArea::load_area_info(void)
{
	TRACE_FUNCATION();
	GG_CommQueryRequest request;
	request.set_templatename(GG_SQL_GAMESVR_LOAD_ROOM_INFO);
	request.add_queryparas((toString(m_area_id)));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMMON_QUERY, MQ_TRANSMIT_DIRECT,
		SITE_NAME_ROOMSVR + "." + Game_Process::instance()->getGameName() + ".master", request, response))
	{
		LOG_ERROR("get room_info file failed");
		return -1;
	}

	if (response.m_result != GG_ERROR_SUCCESS)
	{
		LOG_ERROR("get room info failed failed.result:" + toString(response.m_result));
		return -1;
	}

	const GG_CommQueryResponse& pbresponse = response.ref_pbmsg<GG_CommQueryResponse>();
	if (pbresponse.queryresult().empty())
	{
		LOG_ERROR("get room info empty");
		return -1;
	}
	m_area_info.parse(pbresponse.queryresult(0));
	return 0;
}

int GG_GameArea::area_id(void)
{
	TRACE_FUNCATION();
	return m_area_id;
}

int GG_GameArea::handle_timeout (const ACE_Time_Value &current_time, const void *act)
{
	TRACE_FUNCATION();
	return 0;
}

void GG_GameArea::on_player_enter(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyEnterRoomRequest& request = msg.ref_pbmsg<GG_ApplyEnterRoomRequest>();
	auto iter = m_all_player.find(request.userid());
	LOG_DEBUG("player(" + toString(request.userid()) + "enter room");
	if (iter == m_all_player.end())
	{
		int room_id = 0;
		if (GamePlayer_Manager::instance()->is_player_in_room(request.userid(), room_id))
		{
			// T出卡房用户
			LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") kick out user("
				+ toString(request.userid()) + ") from old room(" + toString(room_id) + ")");
			GG_Request quitRequest;
			quitRequest.m_cmd_type = GG_GameCmd::GG_CMD_APPLY_QUIT_ROOM;
			GG_ApplyQuitRoomRequest quitRoomReq;
			quitRoomReq.set_roomid(room_id);;
			quitRoomReq.set_userid(request.userid());
			quitRequest.m_pPBMsg = &quitRoomReq;
			GameArea_Manager::instance()->dispatch_area_msg(quitRequest);
			on_new_player_enter(msg);
		}
		else
		{
			on_new_player_enter(msg);
		}
	}
	else
	{
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") player(" + toString(request.userid()) + ") reenter");
		iter->second->set_player_name(request.playername());
		iter->second->come_back_game();
		msg.send_failed_reason(GG_ERROR_SUCCESS);
	}
}


void GG_GameArea::on_new_player_enter(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyEnterRoomRequest& request = msg.ref_pbmsg<GG_ApplyEnterRoomRequest>();

	GG_Player *player = add_player(request.userid(), msg.session_id());
	if (NULL == player)
	{
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + 
			") add player(" + toString(request.userid()) + ") failed. GG_ERROR_SYSTEM_ERROR");
		msg.send_failed_reason(GG_ERROR_HAVENO_TOKEN_LOGIN);
		return;
	}
	player->set_player_name(request.playername());
	int idle_seat_number = m_game_desk->get_idle_seat_number();
	if (-1 == idle_seat_number)
	{
		LOG_WARN("room(" + toString(get_area_info().m_area_id) +
			") add player(" + toString(request.userid()) + ") failed. GG_ERROR_HAVE_NO_IDLE_SEAT");
		msg.send_failed_reason(GG_ERROR_HAVE_NO_SEAT);
		m_all_player.erase(request.userid());
		destroy_player(player);
		return;
	}

	GG_INT64 buy_in_count = m_game_desk->get_sitdown_buyin_count();
	if (-1 == player->buy_in(buy_in_count, buy_in_count))
	{
		LOG_WARN("user(" + toString(request.userid()) + " ) buy in failed");
		msg.send_failed_reason(GG_ERROR_USER_BUY_IN_FAILED);
		m_all_player.erase(request.userid());
		destroy_player(player);
		return;
	}

	msg.send_failed_reason(GG_ERROR_SUCCESS);
	
	int seat_num = m_game_desk->player_sit_down(player, idle_seat_number);
	assert(seat_num == idle_seat_number);

}

void GG_GameArea::on_recv_msg(const GG_Message &msg)
{
	TRACE_FUNCATION();
	std::map<CMD_TYPE, GG_GameArea::MSG_HANDLER>::iterator iter = m_msg_handler.find(msg.m_cmd_type);
	if (iter == m_msg_handler.end())
	{
		LOG_ERROR("cmd(" + toString(msg.m_cmd_type) + ") have no handler");
	}
	else
	{
		(this->*(iter->second))(msg);
	}
}

bool GG_GameArea::register_msg_handler(const CMD_TYPE& cmd_type, GG_GameArea::MSG_HANDLER handler)
{
	TRACE_FUNCATION();
	m_msg_handler[cmd_type] = handler;
	return true;
}

bool GG_GameArea::register_rpc_handler(const std::string& rpc_name, GG_GameArea::RPC_HANDLER handler)
{
	TRACE_FUNCATION();
	m_rpc_handler[rpc_name] = handler;
	return true;
}

void GG_GameArea::on_player_exit(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyQuitRoomRequest& request = msg.ref_pbmsg<GG_ApplyQuitRoomRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}

	if (-1 != player->get_seat_number())
	{
		if (-1 == m_game_desk->player_stand_up(player, GG_PlayerStandUpRequest::APPLY_STAND_UP))
		{
			msg.send_failed_reason(GG_ERROR_GAME_NOT_OVER);
			return;
		}
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);

	//broadcast_player_quit(player, GG_PlayerQuitRoom_Request::APPLY_QUIT_ROOM);
	destroy_player(player);
}


GG_Player* GG_GameArea::add_player(int player_id, long session_id)
{
	std::map<int, GG_Player*>::iterator iter = m_all_player.find(player_id);
	if (iter == m_all_player.end())
	{
		GG_Player *new_player = create_player(player_id, session_id);
		if (NULL == new_player)
		{
			LOG_ERROR("room(" + toString(get_area_info().m_area_id) + ")create player(" + toString(player_id) + ") failed");
			return NULL;
		}
		new_player->m_pGameArea = this;
		new_player->m_pGameDesk = m_game_desk;
		m_all_player[player_id] = new_player;
		return new_player;
	}
	return iter->second;
}

GG_Player* GG_GameArea::find_player(int player_id)
{
	std::map<int, GG_Player*>::iterator iter = m_all_player.find(player_id);
	if (iter != m_all_player.end())
	{
		return iter->second;
	}
	return NULL;
}

void GG_GameArea::broadcast_player_seat(int seat_number, GG_Player *player)
{
	TRACE_FUNCATION();
	GG_PlayerSitDownRequest request;
	request.set_userid(player->get_player_id());
	request.set_playername(player->get_player_name());
	request.set_seatnumber(seat_number);
	broadcast_message(GG_CMD_PLAYER_SITDOWN, request);
}

void GG_GameArea::broadcast_player_enter(GG_Player *player)
{
	GG_PlayerEnterRoomRequest request;
	request.set_userid(player->get_player_id());
	broadcast_message(GG_CMD_PLAYER_ENTER_ROOM, request);
}

void GG_GameArea::broadcast_player_quit(GG_Player *player, GG_UINT8 reason)
{
	GG_PlayerQuitRoomRequest request;
	request.set_userid(player->get_player_id());
	broadcast_message(GG_CMD_PLAYER_QUIT_ROOM, request);
}

void GG_GameArea::broadcast_message_direct(const GG_Message &msg)
{
	for (std::map<int, GG_Player*>::iterator iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_request_once(msg);
		}
	}
}

void GG_GameArea::broadcast_message(const GG_Message &msg)
{
	for (std::map<int, GG_Player*>::iterator iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_request(msg);
		}
	}
}

void GG_GameArea::send_msg_to_player(int player_id, const GG_Message& msg)
{
	std::map<int, GG_Player*>::iterator iter = m_all_player.find(player_id);
	if (iter == m_all_player.end())
	{
		return;
	}
	iter->second->send_request(msg);
}

void GG_GameArea::broadcast_message(const GG_Message &msg, const std::vector<int>& recv_user_id)
{
	for (std::vector<int>::const_iterator iter = recv_user_id.begin(); iter != recv_user_id.end(); ++iter)
	{
		send_msg_to_player(*iter, msg);
	}
}

void GG_GameArea::on_apply_seat(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplySeatRequest& request = msg.ref_pbmsg<GG_ApplySeatRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		LOG_WARN("unknown player id(" + toString(request.userid()) + ")");
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}

	player->flush_online_time();

	if (-1 != player->get_seat_number())
	{
		msg.send_failed_reason(GG_ERROR_SUCCESS);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") player(" + toString(player->get_player_id()) + ") have seated on number(" + toString(player->get_seat_number()) + ")");
		return;
	}

	if (!m_game_desk->is_seat_idle(request.seatnumber()))
	{
		msg.send_failed_reason(GG_ERROR_HAVE_NO_IDLE_SEAT);
		LOG_INFO("room(" + toString(get_area_info().m_area_id) + ") seat(" + toString((int)request.seatnumber()) + ") have other player");
		return;
	}

	GG_INT64 buy_in_count = m_game_desk->get_sitdown_buyin_count();

	if (-1 == player->buy_in(buy_in_count, buy_in_count))
	{
		LOG_INFO("player buy in failed");
		msg.send_failed_reason(GG_ERROR_USER_BUY_IN_FAILED);
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	LOG_DEBUG("user(" + toString(player->get_player_id()) + ") apply sit down");
	int seat_num = m_game_desk->player_sit_down(player, request.seatnumber());
	assert(-1 != seat_num);
}

void GG_GameArea::on_apply_standup(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyStandupRequest& request = msg.ref_pbmsg<GG_ApplyStandupRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") unknown player id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	if (-1 != player->get_seat_number())
	{
		LOG_DEBUG("player(" + toString(player->get_player_id()) + ") stand up for on_apply_standup");
		if ( -1 != m_game_desk->player_stand_up(player, GG_PlayerStandUpRequest::APPLY_STAND_UP))
		{
			destroy_player(player);
		}
	}
}

void GG_GameArea::check_player_online(void)
{
	TRACE_FUNCATION();
	for (std::map<int, GG_Player*>::iterator iter = m_all_player.begin(); iter != m_all_player.end(); )
	{
		if (iter->second->is_online())
		{
			iter->second->set_online();
			++iter;
			continue;
		}

		GG_Player *player = iter->second;
		if (player->need_kickout_room() )
		{
			m_all_player.erase(iter++);
			if (-1 != player->get_seat_number())
			{	
				LOG_DEBUG("player(" + toString(player->get_player_id()) + ") stand up for is not online");
				m_game_desk->player_stand_up(player, GG_PlayerStandUpRequest::QUIT_ROOM);
				LOG_INFO("room(" + toString(get_area_info().m_area_id) +
					") kickout user(" + toString(player->get_player_id()) + ") for timeout");
			}
			//broadcast_player_quit(player, GG_PlayerQuitRoom_Request::USER_OFFLINE);
			destroy_player_i(player);
		}

		else
		{
			player->set_offline();
			++iter;
		}
	}
}

const GG_GameArea::AreaInfo& GG_GameArea::get_area_info(void)
{
	return m_area_info;
}

void GG_GameArea::on_apply_flush_room(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyFlushRoomRequest& request = msg.ref_pbmsg<GG_ApplyFlushRoomRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	player->delete_player_message();
	m_game_desk->player_flush_room(player, msg);
	player->recv_broadcast(true);
}

void GG_GameArea::broadcast_message(int cmdType, PBMEssage &msg)
{
	GG_Request request;
	request.m_cmd_type = cmdType;
	request.set_pbmsg(&msg);
	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_request(request);
		}
	}
}

void GG_GameArea::broadcast_message(int cmdType, PBMEssage &msg, const std::vector<int>& recv_user_id)
{
	GG_Request request;
	request.m_cmd_type = cmdType;
	request.set_pbmsg(&msg);
	for (std::vector<int>::const_iterator iter = recv_user_id.begin(); iter != recv_user_id.end(); ++iter)
	{
		send_msg_to_player(*iter, request);
	}
}

void GG_GameArea::broadcast_deal_cards(void)
{
	for (std::map<int, GG_Player*>::iterator iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_deal_cards();
		}
	}
}

GG_GameDesk* GG_GameArea::get_game_desk(void)
{
	return m_game_desk;
}

const GG_GameArea::GameWaitInterval& GG_GameArea::get_game_wait_interval() 
{

		return m_area_info.m_game_wait_interval;
}

void GG_GameArea::on_apply_face_show(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyFaceShowRequest& request = msg.ref_pbmsg<GG_ApplyFaceShowRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();

	GG_PlayerFaceShowRequest broadcast_request;
	broadcast_request.set_userid(player->get_player_id());
	broadcast_request.set_faceid(request.faceid());
	broadcast_message(GG_CMD_PLAYER_FACESHOW, broadcast_request);
}

void GG_GameArea::on_apply_chat(const GG_Message &msg)
{
	const GG_ApplyChatRequest &request = msg.ref_pbmsg<GG_ApplyChatRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();

	GG_PlayerChatRequest broadcast_request;
	broadcast_request.set_isshortcut(request.isshortcut());
	broadcast_request.set_chatcontent(request.chatcontent());
	broadcast_request.set_chatuserid(request.userid());
	broadcast_request.set_chatusername(player->get_player_name());
	broadcast_message(GG_CMD_PLAYER_CHAT, broadcast_request);
}

void GG_GameArea::on_apply_use_props(const GG_Message &msg)
{
	const GG_ApplyUsePropsRequest &request = msg.ref_pbmsg<GG_ApplyUsePropsRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	GG_PlayerUsePropsRequest broadcast_request;
	broadcast_request.set_applyuserid(request.userid());
	broadcast_request.set_acceptuserid(request.acceptuserid());
	broadcast_request.set_propsname(request.propsname());
	broadcast_message(GG_CMD_PLAYER_USE_PROPS, broadcast_request);
}

void GG_GameArea::on_apply_present_chips(const GG_Message &msg)
{
	TRACE_FUNCATION();
	GG_ApplyPresentGiftRequest &request = msg.ref_pbmsg<GG_ApplyPresentGiftRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();

	GG_PlayerPresentGiftRequest breq;
	breq.set_presenterid(request.userid());
	breq.set_giftid(request.giftid());

	breq.mutable_acceptorid()->Swap(request.mutable_acceptorid());
	this->broadcast_message(GG_CMD_PLAYER_PRESENT_GIFT, breq);
}

void GG_GameArea::on_apply_present_gift(const GG_Message &msg)
{
	TRACE_FUNCATION();
	GG_ApplyPresentGiftRequest &request = msg.ref_pbmsg<GG_ApplyPresentGiftRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("have no play id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	GG_PlayerPresentGiftRequest breq;
	breq.set_presenterid(request.userid());
	breq.set_giftid(request.giftid());
	breq.mutable_acceptorid()->Swap(request.mutable_acceptorid());
	this->broadcast_message(GG_CMD_PLAYER_PRESENT_GIFT_NEWINTER, breq);
}

GG_INT64 GG_GameArea::get_game_desk_tips(const GG_INT64 chips)
{
	GG_INT64 tips_count = ( chips * m_area_info.m_tips_persent / 100);
	return tips_count;
}

GG_Card_Dealer *GG_GameArea::make_card_dealer(void)
{
	return new GG_Card_Dealer();
}

GG_GameDesk* GG_GameArea::make_game_desk(void)
{
	GG_GameDesk *pDesk = NULL;
	pDesk = new GG_GameDesk(this);
	return pDesk;
}

void GG_GameArea::deal_card(std::vector<PokerCard> &lstCard, int count)
{
	assert(NULL != m_pCardDealer);
	m_pCardDealer->deal_card(lstCard, count);
}

void GG_GameArea::get_all_card(std::vector<PokerCard> &all_card)
{
	m_pCardDealer->get_all_card(all_card);
}

GG_Player* GG_GameArea::create_player(int player_id, long session_id)
{
	GG_Player* player = GamePlayer_Manager::instance()->make_player(player_id, session_id);
	if (NULL == player)
	{
		return NULL;
	}
	GG_UserEnterRoomRequest dcReq;
	dcReq.set_userid(player_id);
	dcReq.set_roomid(m_area_id);
	dcReq.set_smallblind(m_area_info.m_small_blind);
	MQService::instance()->send_mqrequest(GG_CMD_USER_ENTER_ROOM, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, dcReq);
	return player;
}

void GG_GameArea::destroy_player(GG_Player *player)
{
	m_all_player.erase(player->get_player_id());
	destroy_player_i(player);
}

void GG_GameArea::destroy_player_i(GG_Player *player)
{
	// notify dc
	GG_UserQuitRoomRequest dcReq;
	dcReq.set_userid(player->get_player_id());
	MQService::instance()->send_mqrequest(GG_CMD_USER_QUIT_ROOM, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, dcReq);
	GamePlayer_Manager::instance()->destory_player(player);
}

void GG_GameArea::on_call_comm_rpc(const GG_Message &msg)
{
	const GG_RPCRequest &request = msg.ref_pbmsg<GG_RPCRequest>();
	Json::Value json_req = parse_json_string(request.jsonstring());
	//The request like this:<br> {\"rpc_method\":\"test_method\",\"parameter\":{\"say\":\"hello boy!\"}}
	Json::Value json_resp;
	rpc_call(json_req, json_resp);
	GG_RPCResponse response;
	response.set_jsonstring(json_to_string(json_resp));
	msg.send_response(response);
}

void GG_GameArea::rpc_call(const Json::Value& request, Json::Value &response)
{
	if (request[RPC_METHOD].isNull())
	{
		response[RPC_METHOD] = request[RPC_METHOD];
		response[RPC_RESULT] = "have no rpc method";
		return;
	}
	std::map<std::string, GG_GameArea::RPC_HANDLER>::iterator iter = m_rpc_handler.find(request[RPC_METHOD].asString());
	if (iter == m_rpc_handler.end())
	{
		response[RPC_METHOD] = request[RPC_METHOD];
		response[RPC_RESULT] = "unknown rpc method";
		return;
	}
	(this->*(iter->second))(request, response);
}

void GG_GameArea::on_apply_transmit_message(const GG_Message &msg)
{
	const GG_ApplyTransmitMessageRequest &request = msg.ref_pbmsg<GG_ApplyTransmitMessageRequest>();
	msg.send_failed_reason(GG_ERROR_SUCCESS);

	GG_PlayerTransmitMessageRequest trans_req;
	trans_req.set_transmitmsgtype(request.transmitmsgtype());
	trans_req.set_transmitmsgid(request.transmitmsgid());
	trans_req.set_transmitjsondata(request.transmitjsondata());
	trans_req.set_senduserid(request.userid());

	std::vector<int> vecUser(request.recvuserid().begin(), request.recvuserid().end());

	if (!request.recvuserid().empty())
	{
		broadcast_message(GG_CMD_PLAYER_TRANSMIT_MESSAGE, trans_req, vecUser);
		return;
	}

	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast() && iter->first != trans_req.senduserid())
		{
			iter->second->send_request(GG_CMD_PLAYER_TRANSMIT_MESSAGE, trans_req);
		}
	}

}


void GG_GameArea::on_apply_present_gift_newinter(const GG_Message &msg)
{
	TRACE_FUNCATION();
	GG_ApplyPresentGiftNewInterRequest &request = msg.ref_pbmsg<GG_ApplyPresentGiftNewInterRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("have no play id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	GG_PlayerPresentGiftNewInterRequest breq;
	breq.set_presenterid(request.userid());
	breq.set_giftid(request.giftid());
	breq.set_gifturl(request.gifturl());
	breq.mutable_acceptorid()->Swap(request.mutable_acceptorid());
	this->broadcast_message(GG_CMD_PLAYER_PRESENT_GIFT_NEWINTER, breq);
}

int GG_GameArea::get_playing_count(void)
{
	return m_game_desk->get_seated_count();
}

void GG_GameArea::broadcast_wait_post_cards(const GG_Message &msg_post_card_player, const GG_Message &msg_other, const int seat_num)
{
	for (std::map<int, GG_Player*>::iterator iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{

			if (seat_num == iter->second->get_seat_number())
				iter->second->send_request(msg_post_card_player);
			else
				iter->second->send_request(msg_other);
		}
	}
}


void GG_GameArea::broadcast_player_standup(GG_Player *player, int stand_up_reason)
{
	for (std::map<int, GG_Player*>::iterator iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_player_stand_up(player, stand_up_reason);
		}
	}
}


void GG_GameArea::on_apply_post_card(const GG_Message &msg)
{
	const GG_ApplyPostCardRequest& request = msg.ref_pbmsg<GG_ApplyPostCardRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("have no play id(" + toString(request.userid()) + ")");
		return;
	}
	player->flush_online_time();
	
	m_game_desk->player_post_card(player, msg);
}

void GG_GameArea::on_apply_drop(const GG_Message &msg)
{
	const GG_ApplyDropRequest& request = msg.ref_pbmsg<GG_ApplyDropRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("have no play id(" + toString(request.userid()) + ")");
		return;
	}
	player->flush_online_time();
	
	m_game_desk->player_drop(player, msg);
}

void GG_GameArea::on_apply_declare(const GG_Message &msg)
{
	const GG_ApplyDeclareRequest& request = msg.ref_pbmsg<GG_ApplyDeclareRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("have no play id(" + toString(request.userid()) + ")");
		return;
	}
	player->flush_online_time();
	//msg.send_failed_reason(GG_ERROR_SUCCESS);
	m_game_desk->player_declare(player, msg);
}

void GG_GameArea::on_apply_lose_declare(const GG_Message &msg)
{
	const GG_ApplyLoseDeclareRequest& request = msg.ref_pbmsg<GG_ApplyLoseDeclareRequest>();
	
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("have no play id(" + toString(request.userid()) + ")");
		return;
	}
	player->flush_online_time();
	//msg.send_failed_reason(GG_ERROR_SUCCESS);
	m_game_desk->player_lose_declare(player, msg);
}

void GG_GameArea::on_apply_room_status(const GG_Message &msg)
{
	const GG_ApplyRoomStatusRequest& request = msg.ref_pbmsg<GG_ApplyRoomStatusRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("have no play id(" + toString(request.userid()) + ")");
		return;
	}
	player->flush_online_time();
	GG_ApplyRoomStatusResponse response;
	response.set_status( m_game_desk->get_game_status_for_app());
	msg.send_response(response, GG_ERROR_SUCCESS);
}

void GG_GameArea::on_apply_trustee_off(const GG_Message &msg)
{
	const GG_ApplyTrusteeOffRequest& request = msg.ref_pbmsg<GG_ApplyTrusteeOffRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	m_game_desk->player_trusteeoff(player, msg);
}

void GG_GameArea::on_apply_close_account_box(const GG_Message &msg)
{
	const GG_ApplyCloseAccountBoxRequest& request = msg.ref_pbmsg<GG_ApplyCloseAccountBoxRequest>();
	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		return;
	}

	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	m_game_desk->player_close_account_box(player);
}

void GG_GameArea::broadcast_player_digcard(GG_Player* player, const GG_PlayerDigCardRequest& request)
{
	GG_PlayerDigCardRequest sendreq = request;
	GG_PokerCard card = request.card();
	for (std::map<int, GG_Player*>::iterator iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			if (iter->second != player)
			{
				sendreq.mutable_card()->set_color(0);
				sendreq.mutable_card()->set_point(0);
			}
			else
			{
				sendreq.mutable_card()->CopyFrom(card);
			}
			iter->second->send_request(GG_CMD_PLAYER_DIG_CARD, sendreq);
		}
	}
}


void GG_GameArea::on_apply_dig_card(const GG_Message &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyDigCardRequest& request = msg.ref_pbmsg<GG_ApplyDigCardRequest>();

	GG_Player *player = find_player(request.userid());
	if (NULL == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		LOG_WARN("room(" + toString(get_area_info().m_area_id) + ") have no play id(" + toString(request.userid()) + ")");
		return;
	}
	
	player->flush_online_time();
	m_game_desk->player_dig_card(player, msg);
}