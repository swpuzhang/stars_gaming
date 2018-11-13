#pragma once
#include <memory>
#include <time.h>
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "liblog/Log.h"
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

class GameRoom;
class GameDesk;
class GameSeat;
using GAMEROOM_PTR = std::shared_ptr<GameRoom>;
class Player : public std::enable_shared_from_this<Player>
{
public:

	using SELF_TYPE = Player;
	using PLAYER_PTR = std::shared_ptr<Player>;
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

	Player();
	int get_user_id() const { return m_user_id; }
	void set_player_id(int id) { m_user_id = id; }
	void set_user_name(const std::string& user_name) { m_user_name = user_name; }
	const std::string& get_user_name() const { return m_user_name; }
	void set_status(int status);
	int get_status(void) const;
	std::string get_status_name(int status) const;
	bool recv_broadcast(void) const { return m_is_recv_broadcast; }
	void recv_broadcast(bool is_recv) { m_is_recv_broadcast = is_recv; }
	void session_close();
	void set_session(const SESSION_PTR& session) { m_session = session; }
	const SESSION_PTR& get_session() const { return  m_session; }
	SESSION_PTR& get_session() { return  m_session; }
	Session* get_session_memaddr() const { return  m_session.get(); }
	void flush_online_time() { m_online_check_time = time(NULL); }
	const std::shared_ptr<GameRoom>& get_room() const { return m_room; }
	void set_room(const std::shared_ptr<GameRoom>& room) { m_room = room; }
	std::shared_ptr<GameDesk> get_desk() { return m_desk; }
	void set_desk(const std::shared_ptr<GameDesk>& desk) { m_desk = desk; }
	TY_INT64 buy_in(TY_INT64 buy_count, TY_INT64 min_count);
	TY_INT64 buy_chips(TY_INT64 buy_count, TY_INT64 min_count);
	bool send_request(const MESSAGE_PTR &msg);
	bool send_request(int cmd_type, const PBMESSAGE_PTR &msg);
	TY_INT64 carry_add(TY_INT64 add_count);
	void set_carry(TY_INT64 carry);
	int get_seat_number(void) const;
	bool is_player_trusteed_on(void) const { return m_cur_status & PLAYER_TRUSTEED; }
	const std::shared_ptr<GameSeat>& get_seat(void) const { return  m_seat; }
	void set_trustee_off(void);
	bool is_player_playing(void);
	void send_deal_cards(void);
	bool is_online(void);
	bool is_player_offline();
	void set_online();
	void set_offline();
	bool need_kickout_room(void);
	bool is_seated_player(void){return nullptr != m_seat;}
	void send_player_stand_up(const PLAYER_PTR& player, int reason);
private:
	int m_user_id;
	std::string m_user_name;
	time_t m_online_check_time;
	std::shared_ptr<GameRoom> m_room;
	std::shared_ptr<GameDesk> m_desk;
	std::shared_ptr<GameSeat> m_seat;
	SESSION_PTR m_session;
	TY_UINT32 m_cur_status;
	TY_INT64 m_cur_carry;
	TY_INT64 m_active_chips;
	bool m_is_recv_broadcast;
};