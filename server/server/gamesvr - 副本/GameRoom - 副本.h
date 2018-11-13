#pragma once
#include "libmessage/Message.h"
#include "CardDealer.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include "GG_ErrorCode.pb.h"
#include "GG_HallMsg.pb.h"
#include "GG_HallCmd.pb.h"
#include "GG_SvrCmd.pb.h"
#include "GG_SvrMsg.pb.h"
#include "GG_SystemCmd.pb.h"
#include "GG_SystemMsg.pb.h"
#include "GG_GameMsg.pb.h"
#include "GG_GameCmd.pb.h"
#include "libtools/FunctionBindMacro.h"
using namespace GG_SystemMsg;
using namespace GG_SystemCmd;

using namespace GG_SvrMsg;
using namespace GG_SvrCmd;
using namespace GG_HallCmd;
using namespace GG_HallMsg;
using namespace GG_GameMsg;
using namespace GG_GameCmd;
using namespace GG_ErrorCode;

class Player;
class GameDesk;
class CardDealer;
using PLAYER_PTR = std::shared_ptr<Player>;
using MSG_HANDLER = std::function<void(const MESSAGE_PTR& msg)>;


class TimeInterval
{
public:
	TimeInterval() : m_time_deal_card(8400), m_time_active1(15000), m_time_active2(20000),
		m_time_post_card(500), m_time_shuffle_card(750),
		m_time_drop(1000), m_time_distribute(10000),
		m_time_declare(20000), m_time_account_animat(3000),
		m_time_wait_next_game(5000), m_time_declare_animat(3000) {}
	int m_time_deal_card;
	int m_time_active1;
	int m_time_active2;
	int m_time_post_card;
	int m_time_shuffle_card;
	int m_time_drop;
	int m_time_distribute;
	int m_time_declare;
	int m_time_account_animat;
	int m_time_wait_next_game;
	int m_time_declare_animat;
};

struct RoomInfo
{
	int m_room_id;
	TY_INT64 m_small_blind;
	TY_INT64 m_min_carry;
	TY_INT64 m_max_carry;
	int m_max_playing;
	int m_room_type;
	int m_base_exp_win;
	int m_base_exp_lose;
	int m_tips_persent;
	int m_desk_type;
	TY_INT64 m_draw_tips;
	
};

class GameRoom : public std::enable_shared_from_this<GameRoom>
{
public:
	using SELF_TYPE = GameRoom;
	GameRoom(int room_id);
	void handle_msg(const MESSAGE_PTR& msg);
	int get_room_id() const { return  m_room_id; }
	void open();
	const RoomInfo& get_room_info() const { return m_room_info; }
	const TimeInterval& get_time_interval() const { return m_time_interval; }
	void broadcast_message(int cmd_type, const PBMESSAGE_PTR &msg) const;
	void broadcast_message(int cmd_type, const PBMESSAGE_PTR &msg, const std::vector<int>& recv_user_id);
	void broadcast_player_seat(int seat_number, const PLAYER_PTR& player) const;
	void broadcast_player_standup(const PLAYER_PTR& player, int stand_up_reason);
	void broadcast_player_quit(const PLAYER_PTR& player, int quit_reason);
	void broadcast_player_enter(const PLAYER_PTR& player);
	void broadcast_player_digcard(const PLAYER_PTR& player, const GG_PlayerDigCardRequest& request);
	void broadcast_deal_cards(void);
	std::shared_ptr<GameDesk> get_desk(void) { return m_desk; }
	TY_INT64 get_tips(const TY_INT64 chips);
	void deal_card(std::vector<PokerCard> &lstCard, int count);
	void check_player_online(void);
	const PLAYER_PTR& check_player_exist(int user_id) const;

private:
	void register_room_handle(int cmdtype,  MSG_HANDLER func);
	void on_apply_enter_room(const MESSAGE_PTR& msg);
	void on_new_player_enter(const MESSAGE_PTR &msg);
	void on_apply_seat(const MESSAGE_PTR &msg);
	void on_apply_exit(const MESSAGE_PTR &msg);
	void on_apply_standup(const MESSAGE_PTR &msg);
	void on_apply_flush_room(const MESSAGE_PTR& msg);
	void on_apply_face_show(const MESSAGE_PTR& msg);
	void on_apply_chat(const MESSAGE_PTR& msg);
	void on_apply_use_props(const MESSAGE_PTR &msg);
	void on_apply_present_chips(const MESSAGE_PTR &msg);
	void on_apply_present_gift(const MESSAGE_PTR &msg);
	void on_apply_transmit_message(const MESSAGE_PTR &msg);
	void on_apply_present_gift_newinter(const MESSAGE_PTR &msg);
	void on_apply_post_card(const MESSAGE_PTR &msg);
	void on_apply_dig_card(const MESSAGE_PTR &msg);
	void on_apply_drop(const MESSAGE_PTR &msg);
	void on_apply_declare(const MESSAGE_PTR &msg);
	void on_apply_lose_declare(const MESSAGE_PTR &msg);
	void on_apply_room_status(const MESSAGE_PTR &msg);
	void on_apply_trustee_off(const MESSAGE_PTR &msg);
	void on_apply_close_account_box(const MESSAGE_PTR &msg);
	

	void send_msg_to_player(int player_id, int cmd_type, const PBMESSAGE_PTR &msg);
	const PLAYER_PTR& find_player(int player_id) const;
	PLAYER_PTR add_player(int player_id, const SESSION_PTR& session);
	PLAYER_PTR create_player(int player_id, const SESSION_PTR& session);
	void destroy_player(const PLAYER_PTR& player);
	void destroy_player_i(const PLAYER_PTR&player);
private:
	int m_room_id;
	std::unordered_map<int, PLAYER_PTR> m_all_player;
	std::unordered_map<int, MSG_HANDLER> m_msg_handler;
	std::shared_ptr<GameDesk> m_desk;
	CardDealer* m_card_dealer;
	TimeInterval m_time_interval;
	RoomInfo m_room_info;
	std::unique_ptr<CardDealer> m_card_dealer;
};