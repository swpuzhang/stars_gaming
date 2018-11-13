#ifndef PlayerManager_H__
#define PlayerManager_H__

#include "libtools/Types.h"
#include "libtools/JsonParser.h"
#include "libtools/BaseTool.h"
#include "libmessage/Message.h"
#include <unordered_map>
#include <boost/serialization/singleton.hpp>

class Player;
using PLAYER_PTR = std::shared_ptr<Player>;

class PlayerManager
{
public:
	PlayerManager(void);
	virtual ~PlayerManager(void);

	void on_session_close(const TcpMsgPtr &msg);

	bool player_token_login(const TcpMsgPtr &msg);

	bool is_player_online(int user_id);

	PLAYER_PTR make_player(int player_id, const TcpSessionPtr& session);

	void destory_player(const PLAYER_PTR &player);

	void tell_user_ingame_status(const TcpMsgPtr& msg);

	void get_ingame_user(google::protobuf::RepeatedField<google::protobuf::int32 >* pUsers);

	bool check_player_session(int user_id, const TcpSessionPtr& session);

	void on_kick_out_user(const TcpMsgPtr &msg);

	bool is_player_in_room(int user_id, int &room_id);

    void flush_user_online(const TcpSessionPtr& session);

   // void on_user_response(int user_id, const TcpMsgPtr &request, const TcpMsgPtr &response);

    void output_manager_info(void);

private:
	void add_user_session(int user_id, const TcpSessionPtr& session);

	TcpSessionPtr recv_session(int user_id);

	void send_login_record(int user_id, TY_UINT8 user_action, const std::string& remote_addr);

	bool check_user_token(const std::string& token, const Json& user_token);
	
	void send_player_login_elsewhere(int user_id, const TcpSessionPtr& session, char reason,std::string description="");

	void shutdown_user_session(const TcpSessionPtr& session);

	void tell_user_logout_game(int user_id);

	void tell_user_login_game(int user_id);

private:
    std::unordered_map<int, PLAYER_PTR> m_login_user;     //key: user_id ,所以登陆了的玩家，包含当前短暂离线的玩家， 玩家离线5分钟后删除记录
    std::unordered_map<TcpSession*, PLAYER_PTR> m_online_user;   // key: session_id， 当前所有登陆玩家，当前在线的玩家
};

using PlayerManagerInstance = boost::serialization::singleton<PlayerManager>;

#endif //PlayerManager_H__


