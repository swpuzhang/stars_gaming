#ifndef GG_GAMEAREA_H__
#define GG_GAMEAREA_H__

#include "ace/Event_Handler.h"
#include "ggpp/GG_Message.h"
#include "ggpp/GG_Task_Manager.h"
#include <map>
#include <list>
#include "json/json.h"
#include "GG_ErrorCode.pb.h"
#include "GG_Card_Dealer.h"
#include "GG_SvrCmd.pb.h"
#include "GG_SvrMsg.pb.h"
#include "GG_GameMsg.pb.h"
#include "GG_GameCmd.pb.h"
#include "GG_SystemCmd.pb.h"
#include "GG_SystemMsg.pb.h"
#include "ggpp/GG_SiteName.h"

using namespace GG_SvrMsg;
using namespace GG_SvrCmd;
using namespace GG_GameMsg;
using namespace GG_GameCmd;
using namespace GG_ErrorCode;
using namespace GG_SystemMsg;
using namespace GG_SystemCmd;

class GG_Player;
class GG_GameDesk;
class GG_Card_Dealer;
const int GAME_DESK_LIMITED_PLAYER = 0;
const int GAME_DESK_UNLIMITED_PLAER = 1;

class GG_GameArea : public ACE_Task<ACE_MT_SYNCH>
{
	typedef void (GG_GameArea::*MSG_HANDLER)(const GG_Message &msg);
	typedef void (GG_GameArea::*RPC_HANDLER)(const Json::Value& request, Json::Value &response);

public:
	static int check_online_callback(const GG_Message &request, void *args, const GG_Response &response);

public:
	enum 
	{
		CHECK_ONLINE_INTERVAL = 10,
		FLUSH_ROOM_INTERVAL = 100,
	};

	class GameWaitInterval
	{
	public:
		ACE_Time_Value m_time_deal_card;
		ACE_Time_Value m_time_active1;
		ACE_Time_Value m_time_active2;
		ACE_Time_Value m_time_post_card;
		ACE_Time_Value m_time_shuffle_card;
		ACE_Time_Value m_time_drop;
		ACE_Time_Value m_time_distribute;
		ACE_Time_Value m_time_declare;
		ACE_Time_Value m_time_account_animat;
		ACE_Time_Value m_time_wait_next_game;
		ACE_Time_Value m_time_declare_animat;
		void parse(const GG_DBRow& db_row);
	};

	class AreaInfo
	{
	public:
		int m_area_id;
		GG_INT64 m_small_blind;
		GG_INT64 m_min_carry;
		GG_INT64 m_max_carry;
		int m_max_playing;
		int m_room_type;
		int m_base_exp_win;
		int m_base_exp_lose;
		int m_tips_persent;
		int m_desk_type;
        int m_total_match_score;
		GG_INT64 m_draw_tips;
		GameWaitInterval m_game_wait_interval;

		void parse(const GG_DBRow& db_row);
	};

public:
	GG_GameArea(int id);
	virtual ~GG_GameArea(void);

	virtual int open(void *args = 0);

	virtual int handle_timeout (const ACE_Time_Value &current_time, const void *act = 0);

	int area_id(void);

	void broadcast_message(const GG_Message &msg);

	void broadcast_message(int cmdType, PBMEssage &msg);

	void broadcast_message_direct(const GG_Message &msg);

	void broadcast_message(const GG_Message &msg, const std::vector<int>& recv_user_id);

	void broadcast_message(int cmdType, PBMEssage &msg, const std::vector<int>& recv_user_id);

	const AreaInfo& get_area_info(void);

	void on_recv_msg(const GG_Message &msg);

	void broadcast_deal_cards(void);

	GG_GameDesk* get_game_desk(void);

	const GameWaitInterval& get_game_wait_interval();

	GG_INT64 get_game_desk_tips(const GG_INT64 chips);

	void deal_card(std::vector<PokerCard> &lstCard, int count);

	void get_all_card(std::vector<PokerCard> &all_card);

	void check_player_online(void);

    void destroy_player(GG_Player *player);

    int get_playing_count(void);

	void broadcast_wait_post_cards(const GG_Message &msg_post_card_player, const GG_Message &msg_other, const int seat_num);

	void broadcast_player_seat(int seat_number, GG_Player *player);

	void broadcast_player_standup(GG_Player *player, int stand_up_reason);

	GG_Player* find_player(int player_id);

	void broadcast_player_quit(GG_Player *player, GG_UINT8 reason);

	void broadcast_player_digcard(GG_Player* player, const GG_PlayerDigCardRequest& request);;
protected:
	//////////////////////////////////////////////////////////////////////////
	void on_player_enter(const GG_Message &msg);
	void on_player_exit(const GG_Message &msg);
	void on_apply_seat(const GG_Message &msg);
	void on_apply_standup(const GG_Message &msg);

	void on_apply_flush_room(const GG_Message &msg);
	void on_apply_face_show(const GG_Message &msg);
	void on_apply_chat(const GG_Message &msg);
	void on_apply_use_props(const GG_Message &msg);
	void on_apply_present_chips(const GG_Message &msg);
	void on_apply_present_gift(const GG_Message &msg);
	void on_call_comm_rpc(const GG_Message &msg);
	void on_apply_transmit_message(const GG_Message &msg);
    void on_apply_present_gift_newinter(const GG_Message &msg);
	void on_apply_dig_card(const GG_Message &msg);
	void on_apply_drop(const GG_Message &msg);
	void on_apply_declare(const GG_Message &msg);
	void on_apply_lose_declare(const GG_Message &msg);
	void on_apply_room_status(const GG_Message &msg);
	void on_apply_close_account_box(const GG_Message &msg);

	void rpc_call(const Json::Value& request, Json::Value &response);

	void send_msg_to_player(int player_id, const GG_Message& msg);

    void destroy_player_i(GG_Player *player);


	void on_apply_auto_standup(const GG_Message &msg);

	void on_apply_post_card(const GG_Message &msg);

	void on_apply_trustee_off(const GG_Message &msg);

	bool get_user_room_info(int user_id, int &room_id);
protected:
	int load_area_info(void);

	bool register_msg_handler(const CMD_TYPE& cmd_type, GG_GameArea::MSG_HANDLER handler);

	bool register_rpc_handler(const std::string& rpc_name, GG_GameArea::RPC_HANDLER handler);

	GG_Player* add_player(int player_id, long session_id);

	

	virtual GG_Player* create_player(int player_id, long session_id);

	void broadcast_player_enter(GG_Player *player);
	
	void on_new_player_enter(const GG_Message &msg);

	virtual GG_Card_Dealer *make_card_dealer(void);

	virtual GG_GameDesk* make_game_desk(void);
private:
	int m_area_id;
	AreaInfo m_area_info;
	std::map<CMD_TYPE, GG_GameArea::MSG_HANDLER> m_msg_handler;
	std::map<std::string, GG_GameArea::RPC_HANDLER> m_rpc_handler;
	std::map<int, GG_Player*> m_all_player;
	GG_GameDesk *m_game_desk;
	GG_Card_Dealer *m_pCardDealer;
};


Json::Value parse_json_string(const std::string& json_string);
std::string json_to_string(const Json::Value& json_value);

#endif  //GG_GAMEAREA_H__

