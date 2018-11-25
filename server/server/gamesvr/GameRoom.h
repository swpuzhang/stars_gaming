#pragma once
#include "libmessage/Message.h"
#include "CardDealer.h"
#include <unordered_map>
#include <memory>
#include <functional>
#include "ErrorCode.pb.h"
#include "LobbyMsg.pb.h"
#include "LobbyCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"
#include "libtools/FunctionBindMacro.h"
using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

class Player;
using PLAYER_PTR = std::shared_ptr<Player>;
using MSG_HANDLER = std::function<void(const TcpMsgPtr& msg)>;


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
	void handle_msg(const TcpMsgPtr& msg);
	int get_room_id() const { return  m_room_id; }
	void open();
	void broadcast_message(int cmd_type, const PbMessagePtr &msg) const;
	void broadcast_message(int cmd_type, const PbMessagePtr &msg, const std::vector<int>& recv_user_id);

	
	const PLAYER_PTR check_player_exist(int user_id) const;

private:
	void register_room_handle(int cmdtype,  MSG_HANDLER func);
	void on_apply_enter_room(const TcpMsgPtr& msg);
	
	

	void send_msg_to_player(int player_id, int cmd_type, const PbMessagePtr &msg);
	const PLAYER_PTR find_player(int player_id) const;
	PLAYER_PTR add_player(int player_id, const TcpSessionPtr& session);
	PLAYER_PTR create_player(int player_id, const TcpSessionPtr& session);
	void destroy_player(const PLAYER_PTR& player);
	void destroy_player_i(const PLAYER_PTR&player);
private:
	int m_room_id;
	std::unordered_map<int, PLAYER_PTR> m_all_player;
	std::unordered_map<int, MSG_HANDLER> m_msg_handler;
};