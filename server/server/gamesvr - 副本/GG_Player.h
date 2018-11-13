#ifndef GG_PLAYER_H_
#define GG_PLAYER_H_
#include <string>
#include "GG_GameMsg.pb.h"
using namespace GG_GameMsg;
class GG_Message;
class GG_GameArea;
class GG_Service;


class GG_Player : public ACE_Event_Handler
{
	friend class GG_GameArea;
	friend class GG_GameDesk;
public:
	enum 
	{
		PLAYER_MASK = 0x000F,   //掩码
		PLAYER_TRUSTEED = 0x0080, // 玩家托管状态，注意：托管状态为组合状态。  状态必须通过 & 来获取，而不是 == 来获取。 取托管状态写法，例如：  if (m_status & PLAYER_TRUSTEED)
		PLAYER_OFFLINE = 0x0100, // 玩家不在线，逃跑也算不在线
		PLAYER_ESCAPED = 0x0200, //玩家逃跑


		PLAYER_WAIT_NEXT_GAME = 1, 
		PLAYER_UNREADY,
		PLAYER_PLAYING,		
		PLAYER_DROP, 
		PLAYER_DECLARE,
		PLAYER_STAND_UP,
	};
public:
	GG_Player(void);
	virtual ~GG_Player(void);

    virtual int handle_timeout (const ACE_Time_Value &current_time, const void *act = 0);

	int retry_send_request_i(void);

	int send_request(const GG_Message &msg);

	int send_request(int cmdType, PBMEssage &msg);

    // 一次性立即投递消息，不重传
    int send_request_once(const GG_Message &msg);

    int on_recv_response(const GG_Message &request, const GG_Response &response);

	void set_session_id(long session_id);

	long get_session_id(void);

	void set_player_id(int id);

	bool is_in_room(void);

	int get_player_id(void);

	void set_player_name(const std::string& name);

	const std::string& get_player_name(void);

	void set_status(int status);

	int get_status(void);

	void flush_online_time(void);

	bool is_online(void);

	bool need_kickout_room(void);

	GG_INT64 get_carry(void);

    GG_INT64 get_chips_active(void);

	void set_carry(GG_INT64 carry);

	GG_INT64 carry_add(GG_INT64 add_count);

	GG_INT64 carry_sub(GG_INT64 sub_count);

    void send_deal_cards(void);
	
	void set_game_desk(GG_GameDesk *desk);

    void ready_for_game(void);

	GG_INT64 buy_in(GG_INT64 buy_count, GG_INT64 min_count);

	int present_chips_to_other(GG_INT64 chips_count, GG_Player *other);

	void stand_up(void);

	void sit_down(GG_GameSeat *seat);

	bool recv_broadcast(void);

	void recv_broadcast(bool is_recv);

	bool is_seated_player(void);

	bool is_player_playing(void);

	int  get_seat_number(void);

	GG_GameSeat* get_seat(void);

	void set_platform(const Json::Value& platform);

	const Json::Value& get_platform(void);

	GG_GameArea* get_game_area(void);

    bool check_chips_carry(void);

    void send_user_box_time_recover(void);

    void send_user_box_time_pause(void);

	void set_offline() ;
	void set_online() ;
	bool is_player_offline() ;

	void set_trustee_on(void) ;
	void set_trustee_off(void);
	bool is_player_trusteed_on(void);

	void set_escaped() ;
	void set_unescaped() ;
	bool is_player_escaped() ;

	void escape_game(void);

	void come_back_game(void);

	int add_active_money(const GG_INT64 carray);

	void deal_loser_account(const GG_INT64 all_loser_count, GG_INT64& real_count, GG_INT64& all_chips, int room_id, const std::string& game_id);

	std::string get_status_name(int status) const;

	void send_player_stand_up(GG_Player *player, int reason);

	bool need_set_offline(void);

	bool is_player_need_standup();

	GG_INT64 get_user_money();

	void delete_player_message();

	GG_INT64 get_chips() { return m_active_chips + m_cur_carry; }

	void set_chips(const GG_INT64 current_chips) { m_active_chips = current_chips; }

	int add_chips(GG_INT64 chips, int add_reason);

	int add_carry(GG_INT64 chips, int add_reason);

	int bet_chips(GG_INT64 chips, int add_reason, GG_INT64& real_bet);

	void send_player_quit_room(GG_Player *player, GG_UINT8 reason);
private:
    // 新进入房间购买筹码
    GG_INT64 buy_chips(GG_INT64 buy_count, GG_INT64 min_count);

    // 座位上购买筹码增加携带
    GG_INT64 buy_add_carry_chips(GG_INT64 need_add_carry);

	void set_game_seat(GG_GameSeat *seat);

	void send_game_leave_account(const std::string& game_id = "");

	int send_request_i(void);

   
private:
	int m_player_id;
	std::string m_player_name;
	int m_cur_status;
	time_t m_online_check_time;
	bool m_is_recv_broadcast;
	GG_GameArea *m_pGameArea;
	GG_GameDesk *m_pGameDesk;
};

#endif //GG_PLAYER_H_

