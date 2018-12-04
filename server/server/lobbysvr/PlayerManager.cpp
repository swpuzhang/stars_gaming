#include "PlayerManager.h"
#include "LanchProcess.h"
#include "Player.h"
#include "libtools/JsonParser.h"
#include "libtools/BaseTool.h"
#include "libserver/TcpSession.h"
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

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

PlayerManager::PlayerManager(void)
{
}

PlayerManager::~PlayerManager(void)
{
}

void PlayerManager::on_session_close(const TcpMsgPtr &msg)
{
	TRACE_FUNCATION();
	INFO_LOG << "session(" << msg->session_ptr().get() << ") close";
	auto iter = m_online_user.find(reinterpret_cast<TcpSession*>(msg->session_ptr().get()));
	if (iter != m_online_user.end())
	{
		DEBUG_LOG << "user(" << iter->second->get_user_id() << ") session(" <<
			msg->session_ptr().get() << ") offline";
		//send_login_record(iter->second->get_user_id(), USER_LOGOUT, msg.get_peer_addr());
		PLAYER_PTR player = iter->second;
		player->session_close();
		m_online_user.erase(iter);
		m_login_user.erase(player->get_user_id());
	}
}

bool PlayerManager::player_token_login(const TcpMsgPtr &msg)
{
	TRACE_FUNCATION();
	const UserTokenLoginRequest &request = msg->pbmessage<UserTokenLoginRequest>();
	std::string decode_token = AES_ecb128_decrypt(request.usertoken());
	Json jv = Json::parse(decode_token);
	if (!check_user_token(request.usertoken(), jv))
	{
		LOG_ERROR("check token failed");
		msg->send_failed_reason(CODE_CHECK_GAME_TOKEN_FAILED);
		FULL_MSG(TcpTag::HeaderType, head, ShutDownSessionRequest, pbmsg, TcpTag, sendmsg);
		head->set_cmdtype(CMD_SHUTDOWN_SESSION);
		msg->session_ptr()->send(sendmsg);
		return false;
	}

	int user_id = jv["user_id"].;
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
	INFO_LOG << "user(" << user_id << ") login from host(" << (tcpsession_ptr->get_addr())
		<< ") session(" << msg->session_ptr().get() << ")";

	//send_login_record(user_id, USER_LOGIN, msg.get_peer_addr());
	tell_user_login_lobby(user_id);
	msg->send_failed_reason(CODE_SUCCESS);
	add_user_session(user_id, std::dynamic_pointer_cast<TcpSession>(msg->session_ptr()));
	return true;
}

boost::optional<int> PlayerManager::check_user_msg(const TcpMsgPtr &msg)
{
	TRACE_FUNCATION();
	boost::optional<int> ret;
	INFO_LOG << " check session(" << msg->session_ptr().get() << ")";
	auto iter = m_online_user.find(reinterpret_cast<TcpSession*>(msg->session_ptr().get()));
	if (iter == m_online_user.end())
	{
		return ret;
	}

	ret = iter->second->get_user_id();
	TcpSessionPtr tcpsession_ptr = std::dynamic_pointer_cast<TcpSession>(msg->session_ptr());
	flush_user_online(tcpsession_ptr);
	return ret;
}

void PlayerManager::add_user_session(int user_id, const TcpSessionPtr& session)
{
    TRACE_FUNCATION();
    TcpSessionPtr old_session;
    auto iter = m_login_user.find(user_id);
    if (iter != m_login_user.end())
    {
		old_session = iter->second->get_session_ptr();
        iter->second->set_session(session);
        m_online_user.erase(old_session.get());
        m_online_user[session.get()] = iter->second;
		//iter->second->retry_send_request_i();
        INFO_LOG << "user_id(" << (user_id) << ") relogin session(" << (old_session.get()) << " -> " << session.get() << ")";
    }

    else
    {
		PLAYER_PTR player = std::make_shared<Player>();
        player->set_player_id(user_id);
        player->set_session(session);
        m_login_user[user_id] = player;
        m_online_user[session.get()] = player;
		INFO_LOG << "user_id(" << (user_id) << ") session(" << (old_session.get()) << " -> " << (session.get()) << ")";
    }

    if (nullptr != old_session && old_session != session)
    {
		//send_player_login_elsewhere(user_id, old_session, KICKFOR_LOGIN_ELSEWHERE);
        INFO_LOG << "shutdown user(" << (user_id) << ") session(" << old_session.get() << ")";
        shutdown_user_session(old_session);
    }
}

TcpSessionPtr PlayerManager::recv_session(int user_id)
{
    TRACE_FUNCATION();
	TcpSessionPtr session;
    
	auto iter = m_login_user.find(user_id);
    if (iter != m_login_user.end())
    {
		session = iter->second->get_session_ptr();
    }
    return session;
}

PLAYER_PTR PlayerManager::make_player(int player_id, const TcpSessionPtr& session)
{
	std::shared_ptr<Player> player;
	auto iter = m_login_user.find(player_id);
	if (iter == m_login_user.end())
	{
		INFO_LOG << "new player(" << player_id << ") enter";
		player = std::make_shared<Player>();
		player->set_player_id(player_id);
		player->set_session(session);
		m_login_user[player_id] = player;
		m_online_user[session.get()] = player;
	}
	else
	{
        if (nullptr == iter->second->get_room())
        {
            player = iter->second;
			player->flush_online_time();
        }
        else
        {
            ERROR_LOG << "player(" << (player_id) << ") have been in room(" << (iter->second->get_room()->get_room_id()) + ")";
        }
	}
	return player;
}

void PlayerManager::destory_player(const PLAYER_PTR &player)
{
	INFO_LOG << "destory player(" << (player->get_user_id()) << ")";
   
    //tell_user_logout_game(player->get_user_id());

	auto iter = m_login_user.find(player->get_user_id());
    if (iter != m_login_user.end())
    {
        DEBUG_LOG << "destory user(" << (player->get_user_id()) << ") session(" << 
			(player->get_session_memaddr()) << ") offline";
        m_online_user.erase(iter->second->get_session_ptr().get());
        m_login_user.erase(iter);
    }
    else
    {
        ERROR_LOG << "have no user(" << (player->get_user_id()) << ")";
        assert(0);
    }
}

bool PlayerManager::is_player_online(int user_id)
{
	return recv_session(user_id) != nullptr;
}


bool PlayerManager::check_user_token(const std::string& token, const Json& user_token)
{
	TRACE_FUNCATION();

	if (!user_token.is_valid() || !user_token["user_id"].is_number() || !user_token["user_account"].is_string() || !user_token["user_type"].is_number())
	{
		LOG_ERROR("para invalid");
		return false;
	}
	return true;
}

bool PlayerManager::check_player_session(int user_id, const TcpSessionPtr& session)
{
	bool check_result = false;
	auto iter = m_login_user.find(user_id);
	if (iter != m_login_user.end() && iter->second->get_session_ptr() == session)
	{
		check_result = true;
	}

	return check_result;
}

void PlayerManager::on_kick_out_user(const TcpMsgPtr &msg)
{
	const KickOutUserRequest& request = msg->pbmessage<KickOutUserRequest>();
	TcpSessionPtr recvses = recv_session(request.userid());
	if (nullptr == recvses)
	{
		ERROR_LOG << "NO_SESSION_OF_USERID(" << (request.userid()) << ")";
		return;
	}
	send_player_login_elsewhere(request.userid(), recvses, request.reason(), request.description());
	INFO_LOG << "kick out user(" << (request.userid()) << ") in room(" << (room_id) << ")";
	shutdown_user_session(recvses);
}


void PlayerManager::send_player_login_elsewhere(int user_id, const TcpSessionPtr& session, char reason,std::string description)
{
	WARN_LOG << "user(" << (user_id) << ") LOGIN_ELSEWHERE old sesssion("
		<< session.get() << ")";
	FULL_MSG(TcpTag::HeaderType, head, PlayerLoginElsewhereRequest, pbmsg, TcpTag, request);
	head->set_cmdtype(CMD_PLAYER_LOGIN_ELSEWHERE);
	pbmsg->set_reason(reason);
	pbmsg->set_description(description);
	request->set_session(session);
	session->send(request);
}

void PlayerManager::shutdown_user_session(const TcpSessionPtr& session)
{
    INFO_LOG << "shutdown session(" << session.get() << ")";
	FULL_MSG(TcpTag::HeaderType, head, ShutDownSessionRequest, pbreq, TcpTag, close_req);
	head->set_cmdtype(CMD_SHUTDOWN_SESSION);
	close_req->set_session(session);
	session->send(close_req);
}

void PlayerManager::get_all_user(google::protobuf::RepeatedField<google::protobuf::int32 >* users)
{
	for (auto iter = m_login_user.begin(); iter != m_login_user.end(); ++iter)
	{
		users->Add(iter->first);
	}
}


void PlayerManager::tell_user_login_lobby(int user_id)
{
	NotifyTokenLoginRequest request;
	request.set_userid(user_id);
	LanchProcessInstance::get_mutable_instance().get_mq().send_message(CMD_NOTIFY_TOKEN_LOGIN,
		request, EXCHANGE_LOBBY_TOPIC, ROUTE_TOKEN_LOGIN);
}

void PlayerManager::flush_user_online(const TcpSessionPtr& session)
{
	auto iter = m_online_user.find(session.get());

    if (iter != m_online_user.end())
    {
		DEBUG_LOG << "user(" << iter->second->get_user_id() << ") flush_online_time";
        iter->second->flush_online_time();
    }
    else
    {
        WARN_LOG << "session(" << (session.get()) << ") have no login user";
    }
}


void PlayerManager::output_manager_info(void)
{
    INFO_LOG << "ps-player_manager(" << this << ") user_count(" << (m_login_user.size()) << ") active_count(" << (m_online_user.size()) << ")";
}


