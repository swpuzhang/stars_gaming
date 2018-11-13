#ifndef GG_GAME_DESK_H__
#define GG_GAME_DESK_H__

#include <vector>
#include <queue> 
#include <boost/asio.hpp>
#include "GameSeat.h"
#include "libmessage/Message.h"
#include "CardDealer.h"
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
class GameRoom;
using PLAYER_PTR = std::shared_ptr<Player>;
using GAMEROOM_PTR = std::shared_ptr<GameRoom>;

class GameDesk 
{
	friend class GameRoom;
public:
	enum GameStatus
	{
		GAME_WAIT_MASK			 = 0x80000000,
		GAME_ROUND_MASK			 = 0x0000FFFF,
		GAME_UNREADY			 = 0x00000001,
		GAME_WAIT_PLAYING			 = 0x00000002, 
		GAME_DEALER_CARD           = 0x00000004, 
		GAME_ACTIVE1   = 0x00000008,  
		GAME_ACTIVE2 = 0x00000010,
		GAME_DECLARE = 0x00000020,
		GAME_DISTRIBUTE = 0x00000040,
		GAME_SHUFFLE_CARD = 0x00000080,
	};

	enum DigType
	{
		DIG_BOTTOM,
		DIG_POOL
	};

	enum GameOverType
	{
		NORMAL_OVER,
		DROP_OVER,
		BORNING_OVER,
	};

	enum GameEnterScore
	{
		ENTER_SCORE1 = 15   
	};

public:
	GameDesk(const GAMEROOM_PTR& room);
	virtual ~GameDesk(void);

	void on_wait_round_timeout(const SYSTEM_CODE& err);

	// 用户操作 //////////////////////////////////////////////////////////////////////////
    int player_flush_room(const PLAYER_PTR& player, const MESSAGE_PTR &msg);
	int player_sit_down(const PLAYER_PTR& player, int seat_number);
	int player_stand_up(const PLAYER_PTR& player,  int stand_up_reason = 0);
	void player_close_account_box(const PLAYER_PTR&  player);
    //////////////////////////////////////////////////////////////////////////
    	
    GAMEROOM_PTR& get_room(void) const;

	bool is_wait_active1() { return  (m_game_status & GAME_ACTIVE1); }
	bool is_wait_active2() { return  (m_game_status & GAME_ACTIVE2); }

	bool is_wait_active() { return  is_wait_active1() || is_wait_active2(); }

    bool is_game_over(void);

    bool is_game_can_start(void);

	bool is_distribut() { return m_game_status & GAME_DISTRIBUTE; }

	bool is_wait_game() { return m_game_status & GAME_WAIT_PLAYING; }

    bool is_waiting_for_round(void);

	bool is_declareing() { return m_game_status & GAME_DECLARE; }

	bool is_dealer_card() { return m_game_status & GAME_DEALER_CARD; }

    int get_dealer_index(void);

    virtual int get_start_deal_card_count(void);

	void get_player_deal_order(google::protobuf::RepeatedField<google::protobuf::int32>* order);

    // 坐下的玩家数量
    int get_seated_count(bool include_npc = true);

    int get_ingame_count();

    void get_seat_player(google::protobuf::RepeatedPtrField<GG_PlayerInfo>* player_seat);

	void get_joingame_player(google::protobuf::RepeatedPtrField<PB_IngamePlayer>* player_seat);

	virtual TY_INT64 get_sitdown_buyin_count(void);

	bool is_game_unready();


	unsigned int get_game_status(void);

	void ready_for_next_game();

	void get_seated_player_chips(google::protobuf::RepeatedPtrField<GG_PlayerChips>* playChips);

	void deduct_desk_tips(void);

	PokerCard& get_joker_card() { return m_joker_card; }

	std::vector<PokerCard>& get_pool_card() { return m_pokercard_pool; }

	std::vector<PokerCard>& get_bottom_card() { return m_bottom_cards; }

	void player_trusteeoff(const PLAYER_PTR&  player, const MESSAGE_PTR& msg);

	void player_declare(const PLAYER_PTR&  player, const MESSAGE_PTR& request);

	void player_lose_declare(const PLAYER_PTR&  player, const MESSAGE_PTR& request);

	int get_time_game_begin() { return m_time_game_begin; }

	int notify_room_will_over(int cur_score);

	int get_cur_notify_type() { return m_notify_room_type; };

	void deduct_draw_tips();
protected:
	//////////////////////////////////////////////////////////////////////////
    int check_player(const PLAYER_PTR& player);

	bool is_seat_idle(int seat_number);

	int get_idle_seat_number(void);

	void init_seats(void);
	////////////////////游戏流程////////////////////////////////////
	virtual void start_new_game(void);
	void step_desk_ready(void);
	void clear_desk(void);
	void step_deal_cards(void);
	virtual void step_new_round(void);
	void on_newgame_start(void);
	/////////////////////////////////////////////////////////////////

	// 下一个座位号
	int next_seat_number(int seat_number);

	// 下一个坐着玩家的座位号
	int next_seated_number(int seat_number);

    void set_game_status(unsigned int status);

    //////////// 广播消息///////////////////////////////////////////
	void broadcast_player_standup(const PLAYER_PTR& player, int stand_up_reason);
    /////////////////////////////////////////////////////////////////

	bool is_seated_player(const PLAYER_PTR& player);

	void wait_for_round(unsigned int round, int wait_ms = 0);

	std::string get_status_name(unsigned int status) const;

	virtual void submit_game_result(void);

	std::string make_game_id(void);

    void cancel_wait_round(void);

	void send_flush_user_ingame(void);

    void check_player_chips(void);

    TY_UINT8 get_game_status_for_app(void);

    void send_user_box_time_recover(void);

    void send_user_box_time_pause(void);

	void step_unready();

	void step_playing_begin();

	void set_player_matchover_status();

	void set_ingame_player_over_status();

	bool is_game_wait_over(void);

	void caculate_over_account(GG_GameOverRequest& request);

	void check_player_status();

	void active_player(int extra_sec = 0);

	void step_dealer_seat(void);

	void step_active1_timeout();

	void post_default_card();

	void player_post_card(const PLAYER_PTR&  player, const MESSAGE_PTR & request);
	
	bool is_game_declare() { return m_game_status & GAME_DECLARE; }

	bool check_card(const int point, const PokerCard& card,  PokerCard &pool_card);

	void step_gameover_distrbute(void);

	bool is_active_player(const PLAYER_PTR&  player);

	void notify_player_wait_nextgame(int flag);

	void notify_room_status(int type);

	bool is_game_wait_playing() { return is_waiting_for_round() && (m_game_status & GAME_WAIT_PLAYING); }

	void cancle_notify_timer();

	void player_dig_card(const PLAYER_PTR&  player, const MESSAGE_PTR &msg);

	void player_drop(const PLAYER_PTR&  player, const MESSAGE_PTR &msg);

	void step_rand2_timeout();

	void step_declare_timeout();

	void broadcast_post_card(const PLAYER_PTR&  player, const PokerCard& card);

	void step_player_drop(const PLAYER_PTR&  player);

	void default_decalre(const PLAYER_PTR&  player);

	void step_shuffle_card();

	bool is_all_player_over_account();

	TY_INT8 get_last_ingame_seatnum();

	int next_ingame_seat_number(int number);
protected:
	GAMEROOM_PTR m_room;
	int m_max_seat_num;								// 桌子的最大玩家数量
	std::vector<GameSeat> m_seats;				// 牌桌上的位置
	int m_dealer_index;								// 庄家位置， 每一轮开始的位置号
	TY_UINT32 m_game_status;						// 游戏状态
	boost::asio::deadline_timer m_wait_round_timer;
	boost::asio::deadline_timer m_notify_status_timer;
	GG_GameLogRecord m_game_log_record;
	int m_min_game_player_count;            // 最少参与游戏的玩家数量
	int m_active_seatnum;
	PokerCard m_joker_card;
	std::vector<PokerCard> m_pokercard_pool;  //牌池里面的牌
	std::string m_game_id;
	std::vector<PokerCard> m_bottom_cards;            // 底牌
	time_t m_wait_next_time_begin;
	long m_notify_room_timerid;  //等待下一局开始提前2秒超时，通知roomsvr牌局开始， 防止玩家匹配到已经开始的牌桌
	TY_INT8 m_last_winner_seatnum;
	int m_wait_begin;
	TY_INT64 m_chips_pool;
	TY_INT8 m_apply_declare_seatnum;
	int m_time_game_begin;
	bool m_is_first_pool_card;
	int m_gameover_type;
	int m_notify_room_type;
	int m_wait_game_sec;
	int m_draw_sec;
};

#endif  //GG_GAME_DESK_H__

