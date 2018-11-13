#include "GameRoom.h"
#include "PlayerManager.h"
#include "Player.h"
#include "RoomManager.h"
#include "liblog/Log.h"


GameRoom::GameRoom(int room_id) : m_room_id(room_id)
{
	register_room_handle(CMD_APPLY_ENTER_ROOM, MEMFUN_THIS_HOLDER_BIND1(on_apply_enter_room));
	/*register_cmd_handler(CMD_APPLY_SEAT, MEMFUN_THIS_HOLDER_BIND1(on_apply_seat));
	register_cmd_handler(CMD_APPLY_QUIT_ROOM, MEMFUN_THIS_HOLDER_BIND1(on_apply_exit));

	register_cmd_handler(CMD_APPLY_STANDUP, MEMFUN_THIS_HOLDER_BIND1(on_apply_standup));
	register_cmd_handler(CMD_APPLY_FLUSHROOM, MEMFUN_THIS_HOLDER_BIND1(on_apply_flush_room));
	register_cmd_handler(CMD_APPLY_FACESHOW, MEMFUN_THIS_HOLDER_BIND1(on_apply_face_show));
	register_cmd_handler(CMD_APPLY_CHAT, MEMFUN_THIS_HOLDER_BIND1(on_apply_chat));
	register_cmd_handler(CMD_APPLY_USE_PROPS, MEMFUN_THIS_HOLDER_BIND1(on_apply_use_props));
	register_cmd_handler(CMD_APPLY_PRESENT_CHIPS, MEMFUN_THIS_HOLDER_BIND1(on_apply_present_chips));
	//register_cmd_handler(CMD_COMM_RPC, MEMFUN_THIS_HOLDER_BIND1(on_call_comm_rpc));
	register_cmd_handler(CMD_APPLY_PRESENT_GIFT, MEMFUN_THIS_HOLDER_BIND1(on_apply_present_gift));
	register_cmd_handler(CMD_APPLY_TRANSMIT_MESSAGE, MEMFUN_THIS_HOLDER_BIND1(on_apply_transmit_message));
	register_cmd_handler(CMD_APPLY_PRESENT_GIFT_NEWINTER, MEMFUN_THIS_HOLDER_BIND1(on_apply_present_gift_newinter));
	register_cmd_handler(CMD_APPLY_POST_CARD, MEMFUN_THIS_HOLDER_BIND1(on_apply_post_card));
	register_cmd_handler(CMD_APPLY_DIG_CARD, MEMFUN_THIS_HOLDER_BIND1(on_apply_dig_card));
	register_cmd_handler(CMD_APPLY_DROP, MEMFUN_THIS_HOLDER_BIND1(on_apply_drop));
	register_cmd_handler(CMD_APPLY_DECLARE, MEMFUN_THIS_HOLDER_BIND1(on_apply_declare));
	register_cmd_handler(CMD_APPLY_LOSE_DECLARE, MEMFUN_THIS_HOLDER_BIND1(on_apply_lose_declare));
	register_cmd_handler(CMD_APPLY_ROOM_STATUS, MEMFUN_THIS_HOLDER_BIND1(on_apply_room_status));
	register_cmd_handler(CMD_APPLY_TRUSTEE_OFF, MEMFUN_THIS_HOLDER_BIND1(on_apply_trustee_off));
	register_cmd_handler(CMD_APPLY_CLOSE_ACCOUNT_BOX, MEMFUN_THIS_HOLDER_BIND1(on_apply_close_account_box));*/
}

void GameRoom::register_room_handle(int cmdtype, MSG_HANDLER func)
{
	m_msg_handler[cmdtype] = func;
}

void GameRoom::handle_msg(const TcpMsgPtr& msg)
{
	auto itfunc = m_msg_handler.find(msg->header().cmdtype());
	if (itfunc == m_msg_handler.end())
	{
		WARN_LOG << "cmdtype(" << msg->header().cmdtype() << ") game handle null";
	}
	else
	{
		itfunc->second(msg);
	}
}

void GameRoom::open()
{
	
}

const PLAYER_PTR GameRoom::check_player_exist(int user_id) const
{
	const PLAYER_PTR& player = find_player(user_id);
	if (nullptr == player)
	{
		WARN_LOG << "room(" << m_room_id << ") have no play id(" << user_id << ")";
		return nullptr;
	}
	return player;
}


void GameRoom::broadcast_message(int cmd_type, const PbMessagePtr &msg) const
{
	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_request(cmd_type, msg);
		}
	}
}

void GameRoom::broadcast_message(int cmd_type, const PbMessagePtr &msg, const std::vector<int>& recv_user_id)
{
	for (std::vector<int>::const_iterator iter = recv_user_id.begin(); iter != recv_user_id.end(); ++iter)
	{
		send_msg_to_player(*iter, cmd_type, msg);
	}
}


void GameRoom::send_msg_to_player(int player_id, int cmd_type, const PbMessagePtr &msg)
{
	auto iter = m_all_player.find(player_id);
	if (iter == m_all_player.end())
	{
		return;
	}
	iter->second->send_request(cmd_type, msg);
}

const PLAYER_PTR GameRoom::find_player(int player_id) const
{
	auto iter = m_all_player.find(player_id);
	if (iter != m_all_player.end())
	{
		return iter->second;
	}
	return nullptr;
}




void GameRoom::destroy_player(const PLAYER_PTR& player)
{
	m_all_player.erase(player->get_user_id());
	destroy_player_i(player);
}


void GameRoom::destroy_player_i(const PLAYER_PTR&player)
{
	// notify dc
	//UserQuitRoomRequest dcReq;
	//dcReq.set_userid(player->get_user_id());
	//MQService::instance()->send_mqrequest(CMD_USER_QUIT_ROOM, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, dcReq);
	PlayerManagerInstance::get_mutable_instance().destory_player(player);
}

PLAYER_PTR GameRoom::add_player(int player_id, const TcpSessionPtr& session)
{
	auto iter = m_all_player.find(player_id);
	if (iter == m_all_player.end())
	{
		PLAYER_PTR new_player = create_player(player_id, session);
		if (nullptr == new_player)
		{
			ERROR_LOG << "room(" << m_room_id << ")create player(" << (player_id) << ") failed";
			return nullptr;
		}
		new_player->set_room(shared_from_this());
		m_all_player[player_id] = new_player;
		return new_player;
	}
	return iter->second;
}


PLAYER_PTR GameRoom::create_player(int player_id, const TcpSessionPtr& session)
{
	PLAYER_PTR player = PlayerManagerInstance::get_mutable_instance().make_player(player_id, session);
	if (nullptr == player)
	{
		return nullptr;
	}
	//UserEnterRoomRequest dcReq;
	//dcReq.set_userid(player_id);
	//dcReq.set_roomid(m_area_id);
	//dcReq.set_smallblind(m_area_info.m_small_blind);
	//MQService::instance()->send_mqrequest(CMD_USER_ENTER_ROOM, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, dcReq);
	return player;
}

void GameRoom::on_apply_enter_room(const TcpMsgPtr& msg)
{
	const ApplyEnterRoomRequest& req = msg->pbmessage<ApplyEnterRoomRequest>();

	auto iter = m_all_player.find(req.userid());
	DEBUG_LOG << "user(" << iter->first << ") enter room(" << m_room_id << ")";
}
