#include "GG_GameLogRecord.h"
#include "ace/OS.h"
#include "GG_Player.h"
#include "GG_Game_Process.h"
#include "libggppcc/GG_SiteService.h"
#include "GG_SvrCmd.pb.h"
#include "GG_SvrMsg.pb.h"
#include "libggppcc/GG_MQService.h"
using namespace GG_SvrMsg;
using namespace GG_SvrCmd;

GG_GameLogRecord::GG_GameLogRecord(void)
: m_tips_total(0), m_chips_pool(0)
{
}


GG_GameLogRecord::~GG_GameLogRecord(void)
{

}

void GG_GameLogRecord::set_room_info(const GG_GameArea::AreaInfo &room_info)
{
	m_room_info = room_info;
}

void GG_GameLogRecord::set_game_id(const std::string& game_id)
{
	m_log_info.clear();
	m_log_info["game_id"] = game_id;
	m_log_info["start_time"] = (GG_UINT32)ACE_OS::time(NULL);
	m_log_info["room_info"]["room_id"] = m_room_info.m_area_id;
	m_log_info["room_info"]["small_blind"] = toString(m_room_info.m_small_blind);
	m_log_info["room_info"]["room_type"] = m_room_info.m_room_type;
	m_log_info["room_info"]["desk_type"] = m_room_info.m_desk_type;
	m_game_id = game_id;
	m_tips_total = 0;
}

const std::string& GG_GameLogRecord::get_game_id(void) const
{
	return m_game_id;
}

std::string GG_GameLogRecord::json_to_string(const Json::Value &json_value)
{
	Json::FastWriter writer;  
	return writer.write(json_value);
}

std::string GG_GameLogRecord::get_local_time_string(time_t t)
{
	//////////////////////////////////////////////////////////////////////////
	struct tm tmNow =  Game_Process::instance()->local_time(t);
	char buf[PATH_MAX] = {0};
	ACE_OS::sprintf(buf, "%02d:%02d:%02d", tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);
	return buf;
}

std::string get_local_daytime_string(time_t t)
{
	struct tm tmNow = Game_Process::instance()->local_time(t);
	char buf[PATH_MAX] = {0};
	ACE_OS::sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);
	return buf;
}

void GG_GameLogRecord::set_seat_player(std::vector<GG_GameSeat>& seats)
{
	m_player_id.clear();
    for (int i = 0; i < seats.size(); ++i)
    {
        if (seats[i].is_player_joined() && seats[i].is_seated())
        {
            m_log_info["seat_players"][i]["seat"] = seats[i].get_seat_number();
            m_log_info["seat_players"][i]["player"] = seats[i].get_seated_player()->get_player_id();
            m_log_info["seat_players"][i]["platform"] = seats[i].get_seated_player()->get_platform();
            m_player_id.push_back(seats[i].get_seated_player()->get_player_id());
        }
    }
}

void GG_GameLogRecord::set_game_info(GG_INT64 tips_total, GG_INT64 chips_pool)
{
    m_tips_total = tips_total;
    m_chips_pool = chips_pool;
}

void GG_GameLogRecord::player_blind_chips(std::vector<GG_GameSeat>& seats)
{
    Json::Value blind_chips;
    blind_chips["time"] = get_local_time_string(time(NULL));
    blind_chips["action"] = "blind_chips";

    for (int i = 0; i < seats.size(); ++i)
    {
        if (seats[i].is_seated())
        {
            blind_chips["info"][i]["player"] = seats[i].get_seated_player()->get_player_id();
            blind_chips["info"][i]["blind"] = toString(m_room_info.m_small_blind);
            blind_chips["info"][i]["chips"] = toString(seats[i].get_seated_player()->get_chips());
        }
    }
    m_log_info["steps"].append(blind_chips);
}

void GG_GameLogRecord::deal_cards(std::vector<GG_GameSeat>& seats)
{
	Json::Value deal_card;
	deal_card["time"] = get_local_time_string(time(NULL));
	deal_card["action"] = "deal_card";

    for (int i = 0; i < seats.size(); ++i)
    {
        if (seats[i].is_seated())
        {
            deal_card["info"][i]["player"] = seats[i].get_seated_player()->get_player_id();
            const std::vector<PokerCard>& hand_cards = seats[i].get_hand_cards_total();
            for (size_t j = 0; j < hand_cards.size(); ++j)
            {
                deal_card["info"][i]["card"][j] = hand_cards[j].to_string();
            }
        }
    }

	m_log_info["steps"].append(deal_card);
}

void GG_GameLogRecord::player_post_card(int player_id, const std::deque<PokerCard>& card_pool, bool is_active)
{
    Json::Value post_card;
    post_card["time"] = get_local_time_string(time(NULL));
    if (is_active)
    {
        post_card["action"] = "active";
    }
    else
    {
        post_card["action"] = "passive";
    }

    post_card["player"] = player_id;
    for (size_t i = 0; i < card_pool.size(); ++i)
    {
        post_card["card"][i] = card_pool[i].to_string();
    }

    m_log_info["steps"].append(post_card);
}

void GG_GameLogRecord::player_stand_up(int player_id, int seat_number)
{
	Json::Value standup;
	standup["time"] = get_local_time_string(time(NULL));
	standup["action"] = "standup";
	standup["player"] = player_id;
	standup["seat"] = seat_number;
	m_log_info["steps"].append(standup);
}

void GG_GameLogRecord::player_sit_down(int player_id, int seat_number)
{
	Json::Value sitdown;
	sitdown["time"] = get_local_time_string(time(NULL));
	sitdown["action"] = "sitdown";
	sitdown["player"] = player_id;
	sitdown["seat"] = seat_number;
	m_log_info["steps"].append(sitdown);
}

void GG_GameLogRecord::player_win_game(int player_id, GG_INT64 win_player_chips, GG_INT64 chips)
{
    Json::Value win_game;
    win_game["time"] = get_local_time_string(time(NULL));
    win_game["action"] = "win_game";
    win_game["player"] = player_id;
    win_game["win_pool_chips"] = toString(m_chips_pool);
    win_game["win_player_chips"] = toString(win_player_chips);
    win_game["win_chips"] = toString(m_chips_pool + win_player_chips);
    win_game["chips"] = toString(chips);
    m_log_info["steps"].append(win_game);
}

void GG_GameLogRecord::player_lose_game(int player_id, GG_INT64 lose_chips, GG_INT64 chips)
{
    Json::Value lose_game;
    lose_game["time"] = get_local_time_string(time(NULL));
    lose_game["action"] = "lose_game";
    lose_game["player"] = player_id;
    lose_game["lose_chips"] = toString(lose_chips);
    lose_game["chips"] = toString(chips);
    m_log_info["steps"].append(lose_game);
}

void GG_GameLogRecord::wait_palyer_timeout(int player_id)
{
	Json::Value player_timeout;
	player_timeout["time"] = get_local_time_string(time(NULL));
	player_timeout["action"] = "wait_timeout";
	player_timeout["player"] = player_id;
	m_log_info["steps"].append(player_timeout);
}

void GG_GameLogRecord::game_result(const std::string& win_type, const std::vector<GameResultInfo>& result_info)
{
    Json::Value result;
    result["time"] = get_local_time_string(time(NULL));
    result["action"] = "game_result";
    result["win_type"] = win_type;

    for (int i = 0; i < result_info.size(); ++i)
    {
        result["info"][i]["player"] = result_info[i].m_player_id;
        result["info"][i]["net_increase"] = toString(result_info[i].m_net_increase);
        for (int j = 0; j < result_info[i].left_hand_cards.size(); ++j)
        {
            result["info"][i]["left_card"][j] = result_info[i].left_hand_cards[j].to_string();
        }
    }

    m_log_info["steps"].append(result);
    m_log_info["end_time"] = (GG_UINT32)ACE_OS::time(NULL);
}

void GG_GameLogRecord::submit_game_log(void)
{
	TRACE_FUNCATION();
	GG_GameLogRequest request;
	request.set_roomid(m_room_info.m_area_id);
	request.set_smallblind(m_room_info.m_small_blind);
	request.set_gameid(m_game_id);
	request.set_logtime(time(NULL));
	request.set_tipstotal(m_tips_total);
	request.set_loginfo(json_to_string(m_log_info));
	for (auto &e : m_player_id)
	{
		request.add_playerid(e);
	}
	request.set_chipspool(m_chips_pool);
	request.set_roomtype(m_room_info.m_room_type == 2 ? "高级场" : "普通场");
	MQService::instance()->send_mqrequest(GG_CMD_GAMELOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), request);
}


void GG_GameLogRecord::deduct_desk_tips(int palyer_id, GG_INT64 tips_count)
{
	m_tips_total += tips_count;
	Json::Value player_tips;
	player_tips["time"] = get_local_time_string(time(NULL));
	player_tips["action"] = "deduct_tips";
	player_tips["tips_count"] = toString(tips_count);
	player_tips["player"] = palyer_id;
	m_log_info["steps"].append(player_tips);
}

void GG_GameLogRecord::game_result(const GG_SubmitGameResultRequest& msg)
{
	Json::Value result;
	result["time"] = get_local_time_string(time(NULL));
	result["action"] = "game_result";
	result["tips_total"] = toString(m_tips_total);
	
	int i = 0;
	for (auto iter = msg.gameresult().begin(); iter != msg.gameresult().end(); ++iter, ++i)
	{
		result["player"][i]["id"] = iter->userid();
		result["player"][i]["chips_inc"] = toString(iter->chipsincrease());
		result["player"][i]["status"] = (int)iter->statusmask();
	}
	
	m_log_info["steps"].append(result);
	m_log_info["end_time"] = (GG_UINT32)ACE_OS::time(NULL);
}

void GG_GameLogRecord::recover_changes(GG_INT64 changes_count)
{
	m_tips_total += changes_count;
	Json::Value changes;
	changes["time"] = get_local_time_string(time(NULL));
	changes["action"] = "deduct_tips";
	changes["changes_count"] = toString(changes_count);
	m_log_info["steps"].append(changes);
}

void GG_GameLogRecord::active2_player(int seat_number, int player_id)
{
	Json::Value active;
	active["time"] = get_local_time_string(time(NULL));
	active["action"] = "active2";
	active["seat"] = seat_number;
	active["player"] = player_id;
	m_log_info["steps"].append(active);
}

void GG_GameLogRecord::active1_player(int seat_number, int player_id)
{
	Json::Value active;
	active["time"] = get_local_time_string(time(NULL));
	active["action"] = "active1";
	active["seat"] = seat_number;
	active["player"] = player_id;
	m_log_info["steps"].append(active);
}

void GG_GameLogRecord::wait1_palyer_timeout(int player_id)
{
	Json::Value player_timeout;
	player_timeout["time"] = get_local_time_string(time(NULL));
	player_timeout["action"] = "wait1_timeout";
	player_timeout["player"] = player_id;
	m_log_info["steps"].append(player_timeout);
}


void GG_GameLogRecord::wait2_palyer_timeout(int player_id)
{
	Json::Value player_timeout;
	player_timeout["time"] = get_local_time_string(time(NULL));
	player_timeout["action"] = "wait2_timeout";
	player_timeout["player"] = player_id;
	m_log_info["steps"].append(player_timeout);
}

void GG_GameLogRecord::come_back_game(int player_id, int seat_number)
{
	Json::Value step;
	step["time"] = get_local_time_string(time(NULL));
	step["action"] = "back";
	step["seat"] = seat_number;
	step["player"] = player_id;

	m_log_info["steps"].append(step);
}

void GG_GameLogRecord::player_escape(int player_id, int seat_number)
{
	Json::Value step;
	step["time"] = get_local_time_string(time(NULL));
	step["action"] = "escape";
	step["seat"] = seat_number;
	step["player"] = player_id;

	m_log_info["steps"].append(step);
}

void GG_GameLogRecord::player_drop(int player_id, int seat_number, int score, GG_INT64 chips)
{
	Json::Value step;
	step["time"] = get_local_time_string(time(NULL));
	step["action"] = "drop";
	step["seat"] = seat_number;
	step["player"] = player_id;
	step["score"] = score;
	step["drop_chips"] = toString(chips);
	m_log_info["steps"].append(step);
}

void GG_GameLogRecord::player_declare(const GG_PlayerDeclareRequest& msg)
{
	Json::Value step;
	step["time"] = get_local_time_string(time(NULL));
	step["action"] = "declare";
	step["player"] = msg.userid();
	PokerCard card;
	card.parse_msg_card(msg.declarecard());
	step["declare_card"] = card.to_string();
	for (auto it = msg.cardgroup().begin(); it != msg.cardgroup().end(); ++it)
	{
		Json::Value jv;
		jv["type"] = it->type();
		for (auto it2  = it->cards().begin(); it2 != it->cards().end(); ++it2)
		{
			PokerCard temp_card;
			temp_card.parse_msg_card(*it2);
			jv["card"].append(temp_card.to_string());
		}
		step["card_group"].append(jv);
	}
	m_log_info["steps"].append(step);
}

void GG_GameLogRecord::loser_declare(const GG_PlayerLoseDeclareRequest& msg)
{
	Json::Value step;
	step["time"] = get_local_time_string(time(NULL));
	step["action"] = "loser_declare";
	step["player"] = msg.userid();
	step["score"] = msg.score();
	step["chips"] = toString(msg.chips());
	for (auto it = msg.cardgroup().begin(); it != msg.cardgroup().end(); ++it)
	{
		Json::Value jv;
		jv["type"] = it->type();
		for (auto it2 = it->cards().begin(); it2 != it->cards().end(); ++it2)
		{
			PokerCard temp_card;
			temp_card.parse_msg_card(*it2);
			jv["card"].append(temp_card.to_string());
		}
		step["card_group"].append(jv);
	}
	m_log_info["steps"].append(step);
}

void GG_GameLogRecord::dig_card(const GG_PlayerDigCardRequest& msg)
{
	Json::Value jv_digcard;
	jv_digcard["time"] = get_local_time_string(time(NULL));
	jv_digcard["action"] = "dig_card";
	jv_digcard["player"] = msg.userid();
	PokerCard card;
	card.parse_msg_card(msg.card());
	jv_digcard["card"] = card.to_string();
	jv_digcard["dig_type"] = msg.digtype();
	m_log_info["steps"].append(jv_digcard);
}

void GG_GameLogRecord::post_card(const GG_PlayerPostCardRequest& msg)
{
	Json::Value jv_postcard;
	jv_postcard["time"] = get_local_time_string(time(NULL));
	jv_postcard["action"] = "post_card";
	jv_postcard["player"] = msg.userid();
	PokerCard card;
	card.parse_msg_card(msg.card());
	jv_postcard["card"] = card.to_string();

	m_log_info["steps"].append(jv_postcard);
}