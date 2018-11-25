#pragma once
#include <memory>
#include <time.h>
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "liblog/Log.h"
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
#include "libmessage/Message.h"
using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

class GameRoom;

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
	void set_session(const TcpSessionPtr& session) { m_session = session; }
	const TcpSessionPtr& get_session_ptr() const { return  m_session; }
	TcpSessionPtr& get_session_ptr() { return  m_session; }
	TcpSession* get_session_memaddr() const { return  m_session.get(); }
	void flush_online_time() { m_online_check_time = time(NULL); }
	const std::shared_ptr<GameRoom>& get_room() const { return m_room; }
	void set_room(const std::shared_ptr<GameRoom>& room) { m_room = room; }
	bool send_request(const TcpMsgPtr &msg);
	bool send_request(int cmd_type, const PbMessagePtr &msg);
	
private:
	int m_user_id;
	std::string m_user_name;
	time_t m_online_check_time;
	std::shared_ptr<GameRoom> m_room;
	TcpSessionPtr m_session;
	int m_cur_status;
	bool m_is_recv_broadcast;
};