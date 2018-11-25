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

class Player : public std::enable_shared_from_this<Player>
{
public:

	using SELF_TYPE = Player;
	using PLAYER_PTR = std::shared_ptr<Player>;

	Player();
	int get_user_id() const { return m_user_id; }
	void set_player_id(int id) { m_user_id = id; }
	void set_user_name(const std::string& user_name) { m_user_name = user_name; }
	const std::string& get_user_name() const { return m_user_name; }
	void session_close();
	void set_session(const TcpSessionPtr& session) { m_session = session; }
	const TcpSessionPtr& get_session_ptr() const { return  m_session; }
	TcpSessionPtr& get_session_ptr() { return  m_session; }
	TcpSession* get_session_memaddr() const { return  m_session.get(); }
	void flush_online_time() { m_online_check_time = time(NULL); }
	bool send_request(const TcpMsgPtr &msg);
	bool send_request(int cmd_type, const PbMessagePtr &msg);
	
private:
	int m_user_id;
	std::string m_user_name;
	time_t m_online_check_time;
	TcpSessionPtr m_session;
	bool m_is_recv_broadcast;
};