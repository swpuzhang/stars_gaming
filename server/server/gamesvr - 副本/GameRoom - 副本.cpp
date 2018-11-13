#include "GameRoom.h"
#include "PlayerManager.h"
#include "Player.h"
#include "GameDesk.h"
#include "RoomManager.h"
#include "liblog/Log.h"


GameRoom::GameRoom(int room_id) : m_room_id(room_id)
{
	register_room_handle(GG_CMD_APPLY_ENTER_ROOM, MEMFUN_THIS_HOLDER_BIND1(on_apply_enter_room));
	register_cmd_handler(GG_CMD_APPLY_SEAT, MEMFUN_THIS_HOLDER_BIND1(on_apply_seat));
	register_cmd_handler(GG_CMD_APPLY_QUIT_ROOM, MEMFUN_THIS_HOLDER_BIND1(on_apply_exit));

	register_cmd_handler(GG_CMD_APPLY_STANDUP, MEMFUN_THIS_HOLDER_BIND1(on_apply_standup));
	register_cmd_handler(GG_CMD_APPLY_FLUSHROOM, MEMFUN_THIS_HOLDER_BIND1(on_apply_flush_room));
	register_cmd_handler(GG_CMD_APPLY_FACESHOW, MEMFUN_THIS_HOLDER_BIND1(on_apply_face_show));
	register_cmd_handler(GG_CMD_APPLY_CHAT, MEMFUN_THIS_HOLDER_BIND1(on_apply_chat));
	register_cmd_handler(GG_CMD_APPLY_USE_PROPS, MEMFUN_THIS_HOLDER_BIND1(on_apply_use_props));
	register_cmd_handler(GG_CMD_APPLY_PRESENT_CHIPS, MEMFUN_THIS_HOLDER_BIND1(on_apply_present_chips));
	//register_cmd_handler(GG_CMD_COMM_RPC, MEMFUN_THIS_HOLDER_BIND1(on_call_comm_rpc));
	register_cmd_handler(GG_CMD_APPLY_PRESENT_GIFT, MEMFUN_THIS_HOLDER_BIND1(on_apply_present_gift));
	register_cmd_handler(GG_CMD_APPLY_TRANSMIT_MESSAGE, MEMFUN_THIS_HOLDER_BIND1(on_apply_transmit_message));
	register_cmd_handler(GG_CMD_APPLY_PRESENT_GIFT_NEWINTER, MEMFUN_THIS_HOLDER_BIND1(on_apply_present_gift_newinter));
	register_cmd_handler(GG_CMD_APPLY_POST_CARD, MEMFUN_THIS_HOLDER_BIND1(on_apply_post_card));
	register_cmd_handler(GG_CMD_APPLY_DIG_CARD, MEMFUN_THIS_HOLDER_BIND1(on_apply_dig_card));
	register_cmd_handler(GG_CMD_APPLY_DROP, MEMFUN_THIS_HOLDER_BIND1(on_apply_drop));
	register_cmd_handler(GG_CMD_APPLY_DECLARE, MEMFUN_THIS_HOLDER_BIND1(on_apply_declare));
	register_cmd_handler(GG_CMD_APPLY_LOSE_DECLARE, MEMFUN_THIS_HOLDER_BIND1(on_apply_lose_declare));
	register_cmd_handler(GG_CMD_APPLY_ROOM_STATUS, MEMFUN_THIS_HOLDER_BIND1(on_apply_room_status));
	register_cmd_handler(GG_CMD_APPLY_TRUSTEE_OFF, MEMFUN_THIS_HOLDER_BIND1(on_apply_trustee_off));
	register_cmd_handler(GG_CMD_APPLY_CLOSE_ACCOUNT_BOX, MEMFUN_THIS_HOLDER_BIND1(on_apply_close_account_box));
}

void GameRoom::register_room_handle(int cmdtype, MSG_HANDLER func)
{
	m_msg_handler[cmdtype] = func;
}

void GameRoom::handle_msg(const MESSAGE_PTR& msg)
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
	//load_room_info
	m_card_dealer = std::make_shared<CardDealer>();
	m_desk = std::make_shared<GameDesk>(shared_from_this());
}

const PLAYER_PTR& GameRoom::check_player_exist(int user_id) const
{&
	const PLAYER_PTR& player = find_player(user_id);
	if (nullptr == player)
	{
		WARN_LOG << "room(" << m_room_id << ") have no play id(" << user_id << ")";
		return nullptr;
	}
	return player;
}

void GameRoom::on_apply_close_account_box(const MESSAGE_PTR &msg)
{
	const GG_ApplyCloseAccountBoxRequest& request = msg->message<GG_ApplyCloseAccountBoxRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
	}

	msg->send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	m_desk->player_close_account_box(player);
}

void GameRoom::on_apply_trustee_off(const MESSAGE_PTR &msg)
{
	const GG_ApplyTrusteeOffRequest& request = msg->message<GG_ApplyTrusteeOffRequest>();

	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	m_desk->player_trusteeoff(player, msg);
}

void GameRoom::on_apply_room_status(const MESSAGE_PTR &msg)
{
	const GG_ApplyRoomStatusRequest& request = msg->message<GG_ApplyRoomStatusRequest>();

	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
	}

	player->flush_online_time();
	GG_ApplyRoomStatusResponse response;
	response.set_status(m_desk->get_game_status_for_app());
	msg->send_response(GG_ERROR_SUCCESS, response);
}

void GameRoom::on_apply_lose_declare(const MESSAGE_PTR &msg)
{
	const GG_ApplyLoseDeclareRequest& request = msg->message<GG_ApplyLoseDeclareRequest>();

	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	player->flush_online_time();
	//msg.send_failed_reason(GG_ERROR_SUCCESS);
	m_desk->player_lose_declare(player, msg);
}

void GameRoom::on_apply_declare(const MESSAGE_PTR &msg)
{
	const GG_ApplyDeclareRequest& request = msg->message<GG_ApplyDeclareRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	player->flush_online_time();
	//msg.send_failed_reason(GG_ERROR_SUCCESS);
	m_desk->player_declare(player, msg);
}

void GameRoom::on_apply_drop(const MESSAGE_PTR &msg)
{
	const GG_ApplyDropRequest& request = msg->message<GG_ApplyDropRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	player->flush_online_time();

	m_desk->player_drop(player, msg);
}

void GameRoom::on_apply_dig_card(const MESSAGE_PTR &msg)
{
	const GG_ApplyDigCardRequest& request = msg->message<GG_ApplyDigCardRequest>();

	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}

	player->flush_online_time();
	m_desk->player_dig_card(player, msg);
}

void GameRoom::on_apply_post_card(const MESSAGE_PTR &msg)
{
	const GG_ApplyPostCardRequest& request = msg->message<GG_ApplyPostCardRequest>();

	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}

	player->flush_online_time();
	m_desk->player_post_card(player, msg);
}


void GameRoom::on_apply_present_gift_newinter(const MESSAGE_PTR &msg)
{
	GG_ApplyPresentGiftNewInterRequest &request = msg->message<GG_ApplyPresentGiftNewInterRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	GG_PlayerPresentGiftNewInterRequest breq;
	breq.set_presenterid(request.userid());
	breq.set_giftid(request.giftid());
	breq.set_gifturl(request.gifturl());
	breq.mutable_acceptorid()->Swap(request.mutable_acceptorid());
	this->broadcast_message(GG_CMD_PLAYER_PRESENT_GIFT_NEWINTER, breq);
}

void GameRoom::on_apply_transmit_message(const MESSAGE_PTR &msg)
{
	const GG_ApplyTransmitMessageRequest &request = msg->message<GG_ApplyTransmitMessageRequest>();
	msg->send_failed_reason(GG_ERROR_SUCCESS);

	GG_PlayerTransmitMessageRequest trans_req;
	trans_req.set_transmitmsgtype(request.transmitmsgtype());
	trans_req.set_transmitmsgid(request.transmitmsgid());
	trans_req.set_transmitjsondata(request.transmitjsondata());
	trans_req.set_senduserid(request.userid());

	std::vector<int> vecUser(request.recvuserid().begin(), request.recvuserid().end());

	if (!request.recvuserid().empty())
	{
		broadcast_message(GG_CMD_PLAYER_TRANSMIT_MESSAGE, trans_req, vecUser);
		return;
	}

	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast() && iter->first != trans_req.senduserid())
		{
			iter->second->send_request(GG_CMD_PLAYER_TRANSMIT_MESSAGE, trans_req);
		}
	}
}

void GameRoom::on_apply_present_gift(const MESSAGE_PTR &msg) 
{
	TRACE_FUNCATION();
	GG_ApplyPresentGiftRequest &request = msg->message<GG_ApplyPresentGiftRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	GG_PlayerPresentGiftRequest breq;
	breq.set_presenterid(request.userid());
	breq.set_giftid(request.giftid());
	breq.mutable_acceptorid()->Swap(request.mutable_acceptorid());
	this->broadcast_message(GG_CMD_PLAYER_PRESENT_GIFT, breq);
}


void GameRoom::on_apply_present_chips(const MESSAGE_PTR &msg)
{
	TRACE_FUNCATION();
	GG_ApplyPresentGiftRequest &request = msg->message<GG_ApplyPresentGiftRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();

	GG_PlayerPresentGiftRequest breq;
	breq.set_presenterid(request.userid());
	breq.set_giftid(request.giftid());

	breq.mutable_acceptorid()->Swap(request.mutable_acceptorid());
	this->broadcast_message(GG_CMD_PLAYER_PRESENT_GIFT, breq);
}

void GameRoom::on_apply_use_props(const MESSAGE_PTR &msg)
{
	const GG_ApplyUsePropsRequest &request = msg->message<GG_ApplyUsePropsRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);

	player->flush_online_time();
	GG_PlayerUsePropsRequest broadcast_request;
	broadcast_request.set_applyuserid(request.userid());
	broadcast_request.set_acceptuserid(request.acceptuserid());
	broadcast_request.set_propsname(request.propsname());
	broadcast_message(GG_CMD_PLAYER_USE_PROPS, broadcast_request);
}

void GameRoom::on_apply_chat(const MESSAGE_PTR& msg)
{
	const GG_ApplyChatRequest &request = msg->message<GG_ApplyChatRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}

	msg->send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();

	GG_PlayerChatRequest broadcast_request;
	broadcast_request.set_isshortcut(request.isshortcut());
	broadcast_request.set_chatcontent(request.chatcontent());
	broadcast_request.set_chatuserid(request.userid());
	broadcast_request.set_chatusername(player->get_player_name());
	broadcast_message(GG_CMD_PLAYER_CHAT, broadcast_request);
}

void GameRoom::on_apply_face_show(const MESSAGE_PTR& msg)
{
	TRACE_FUNCATION();
	const GG_ApplyFaceShowRequest& request = msg->message<GG_ApplyFaceShowRequest>();
	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();

	GG_PlayerFaceShowRequest broadcast_request;
	broadcast_request.set_userid(player->get_player_id());
	broadcast_request.set_faceid(request.faceid());
	broadcast_message(GG_CMD_PLAYER_FACESHOW, broadcast_request);
}


void GameRoom::on_apply_flush_room(const MESSAGE_PTR& msg)
{
	TRACE_FUNCATION();
	const GG_ApplyFlushRoomRequest& request = msg->message<GG_ApplyFlushRoomRequest>();

	PLAYER_PTR player = check_player_exist(request.userid());
	if (!player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	//player->delete_player_message();
	m_desk->player_flush_room(player, msg);
	player->recv_broadcast(true);
}

void GameRoom::on_apply_enter_room(const MESSAGE_PTR& msg)
{
	const GG_ApplyEnterRoomRequest& req = msg->message<GG_ApplyEnterRoomRequest>();
	
	auto iter = m_all_player.find(req.userid());
	DEBUG_LOG << "user(" << iter->first << ") enter room(" << m_room_id << ")";
	if (iter == m_all_player.end())
	{
		int room_id = 0;
		if (PlayerManagerInstance::get_mutable_instance()->is_player_in_room(req.userid(), room_id))
		{
			// T出卡房用户
			WARN_LOG << "room(" << m_room_id << ") kick out user("
				<< req.userid() << ") from old room(" << room_id <<  ")";
			MAKE_FULL_MSG(head, GG_ApplyQuitRoomRequest, pbreq, quit_room_req);
			head.set_cmdtype(GG_GameCmd::GG_CMD_APPLY_QUIT_ROOM);
			pbreq.set_roomid(room_id);;
			pbreq.set_userid(req.userid());
			RoomManagerInstance::get_mutable_instance().dispach_room_msg(quit_room_req);
			on_new_player_enter(msg);
		}
		else
		{
			on_new_player_enter(msg);
		}
	}
	else
	{
		WARN_LOG << "room(" << m_room_id << ") user(" << (req.userid()) << ") reenter";
		//iter->second->set_player_name(request.playername());
		//iter->second->come_back_game();
		msg->send_failed_reason(GG_ERROR_SUCCESS);
	}
}


void GameRoom::on_apply_seat(const MESSAGE_PTR &msg)
{
	TRACE_FUNCATION();
	const GG_ApplySeatRequest& request = msg->message<GG_ApplySeatRequest>();

	const PLAYER_PTR&  player = find_player(request.userid());
	if (nullptr == player)
	{
		WARN_LOG << "unknown player id(" << (request.userid()) << ")";
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}

	player->flush_online_time();

	if (-1 != player->get_seat_number())
	{
		msg->send_failed_reason(GG_ERROR_SUCCESS);
		WARN_LOG << "room(" << m_room_id << ") user(" << (player->get_user_id()) << ") have seated on number(" << (player->get_seat_number()) << ")";
		return;
	}

	if (!m_desk->is_seat_idle(request.seatnumber()))
	{
		msg->send_failed_reason(GG_ERROR_HAVE_NO_IDLE_SEAT);
		INFO_LOG << "room(" << m_room_id << ") seat(" << ((int)request.seatnumber()) << ") have other player";
		return;
	}

	TY_INT64 buy_in_count = m_desk->get_sitdown_buyin_count();

	if (-1 == player->buy_in(buy_in_count, buy_in_count))
	{
		INFO_LOG << "player buy in failed";
		msg->send_failed_reason(GG_ERROR_USER_BUY_IN_FAILED);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	DEBUG_LOG << "user(" << (player->get_user_id()) << ") apply sit down";
	int seat_num = m_desk->player_sit_down(player, request.seatnumber());
	assert(-1 != seat_num);
}


void GameRoom::on_apply_exit(const MESSAGE_PTR &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyQuitRoomRequest& request = msg->message<GG_ApplyQuitRoomRequest>();

	PLAYER_PTR player = find_player(request.userid());
	if (nullptr == player)
	{
		msg->send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		return;
	}

	if (-1 != player->get_seat_number())
	{
		if (-1 == m_desk->player_stand_up(player, GG_PlayerStandUpRequest::APPLY_STAND_UP))
		{
			msg->send_failed_reason(GG_ERROR_GAME_NOT_OVER);
			return;
		}
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);

	//broadcast_player_quit(player, GG_PlayerQuitRoom_Request::APPLY_QUIT_ROOM);
	destroy_player(player);
}


void GameRoom::broadcast_message(int cmd_type, const PBMESSAGE_PTR &msg) const
{
	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_request(cmd_type, msg);
		}
	}
}

void GameRoom::broadcast_message(int cmd_type, const PBMESSAGE_PTR &msg, const std::vector<int>& recv_user_id)
{
	for (std::vector<int>::const_iterator iter = recv_user_id.begin(); iter != recv_user_id.end(); ++iter)
	{
		send_msg_to_player(*iter, cmd_type, msg);
	}
}


void GameRoom::send_msg_to_player(int player_id, int cmd_type, const PBMESSAGE_PTR &msg)
{
	auto iter = m_all_player.find(player_id);
	if (iter == m_all_player.end())
	{
		return;
	}
	iter->second->send_request(cmd_type, msg);
}

const PLAYER_PTR& GameRoom::find_player(int player_id) const
{
	auto iter = m_all_player.find(player_id);
	if (iter != m_all_player.end())
	{
		return iter->second;
	}
	return nullptr;
}

void GameRoom::on_new_player_enter(const MESSAGE_PTR &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyEnterRoomRequest& request = msg->message<GG_ApplyEnterRoomRequest>();

	PLAYER_PTR player = add_player(request.userid(), msg->get_sender());
	if (nullptr == player)
	{
		WARN_LOG << "room(" << (m_room_id) <<
			") add player(" << (request.userid()) << ") failed. GG_ERROR_SYSTEM_ERROR";
		msg->send_failed_reason(GG_ERROR_HAVENO_TOKEN_LOGIN);
		return;
	}
	player->set_user_name(request.playername());
	int idle_seat_number = m_desk->get_idle_seat_number();
	if (-1 == idle_seat_number)
	{
		WARN_LOG << "room(" << m_room_id << ") add player(" << (request.userid()) +<< ") failed. GG_ERROR_HAVE_NO_IDLE_SEAT";
		msg->send_failed_reason(GG_ERROR_HAVE_NO_SEAT);
		m_all_player.erase(request.userid());
		destroy_player(player);
		return;
	}

	TY_INT64 buy_in_count = m_desk->get_sitdown_buyin_count();
	if (-1 == player->buy_in(buy_in_count, buy_in_count))
	{
		WARN_LOG << "user(" << (request.userid()) << " ) buy in failed";
		msg->send_failed_reason(GG_ERROR_USER_BUY_IN_FAILED);
		m_all_player.erase(request.userid());
		destroy_player(player);
		return;
	}

	msg->send_failed_reason(GG_ERROR_SUCCESS);

	int seat_num = m_desk->player_sit_down(player, idle_seat_number);
	assert(seat_num == idle_seat_number);

}


void GameRoom::on_apply_standup(const MESSAGE_PTR &msg)
{
	TRACE_FUNCATION();
	const GG_ApplyStandupRequest& request = msg->message<GG_ApplyStandupRequest>();

	PLAYER_PTR player = find_player(request.userid());
	if (nullptr == player)
	{
		msg.send_failed_reason(GG_ERROR_USER_NOT_IN_ROOM);
		WARN_LOG << "room(" << (m_room_id) << ") unknown player id(" << (request.userid()) << ")";
		return;
	}
	msg.send_failed_reason(GG_ERROR_SUCCESS);
	player->flush_online_time();
	if (-1 != player->get_seat_number())
	{
		DEBUG_LOG << "player(" << (player->get_user_id()) << ") stand up for on_apply_standup";
		if (-1 != m_desk->player_stand_up(player, GG_PlayerStandUpRequest::APPLY_STAND_UP))
		{
			destroy_player(player);
		}
	}
}

void GameRoom::destroy_player(const PLAYER_PTR& player)
{
	m_all_player.erase(player->get_user_id());
	destroy_player_i(player);
}


void GameRoom::destroy_player_i(const PLAYER_PTR&player)
{
	// notify dc
	//GG_UserQuitRoomRequest dcReq;
	//dcReq.set_userid(player->get_user_id());
	//MQService::instance()->send_mqrequest(GG_CMD_USER_QUIT_ROOM, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, dcReq);
	PlayerManagerInstance::get_mutable_instance().destory_player(player);
}

PLAYER_PTR GameRoom::add_player(int player_id, const SESSION_PTR& session)
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
		new_player->set_desk(m_desk);
		m_all_player[player_id] = new_player;
		return new_player;
	}
	return iter->second;
}


PLAYER_PTR GameRoom::create_player(int player_id, const SESSION_PTR& session)
{
	PLAYER_PTR player = PlayerManagerInstance::get_mutable_instance().make_player(player_id, session);
	if (nullptr == player)
	{
		return nullptr;
	}
	//GG_UserEnterRoomRequest dcReq;
	//dcReq.set_userid(player_id);
	//dcReq.set_roomid(m_area_id);
	//dcReq.set_smallblind(m_area_info.m_small_blind);
	//MQService::instance()->send_mqrequest(GG_CMD_USER_ENTER_ROOM, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, dcReq);
	return player;
}

void GameRoom::broadcast_player_seat(int seat_number, const PLAYER_PTR& player) const
{
	TRACE_FUNCATION();
	GG_PlayerSitDownRequest request;
	request.set_userid(player->get_user_id());
	request.set_playername(player->get_user_name());
	request.set_seatnumber(seat_number);
	broadcast_message(GG_CMD_PLAYER_SITDOWN, request);
}


void GameRoom::broadcast_player_standup(const PLAYER_PTR& player, int stand_up_reason)
{
	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_player_stand_up(player, stand_up_reason);
		}
	}
}


void GameRoom::broadcast_player_quit(const PLAYER_PTR& player, int quit_reason)
{
	GG_PlayerQuitRoomRequest request;
	request.set_userid(player->get_player_id());
	request.set_quitreason(quit_reason);
	broadcast_message(GG_CMD_PLAYER_QUIT_ROOM, request);
}

void GameRoom::broadcast_deal_cards(void)
{
	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			iter->second->send_deal_cards();
		}
	}
}


TY_INT64 GameRoom::get_tips(const TY_INT64 chips)
{
	TY_INT64 tips_count = (chips * get_room_info().m_tips_persent / 100);
	return tips_count;
}

void GameRoom::deal_card(std::vector<PokerCard> &lstCard, int count)
{
	m_card_dealer->deal_card(lstCard, count);
}


void GameRoom::check_player_online()
{
	TRACE_FUNCATION();
	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); )
	{
		if (iter->second->is_online())
		{
			iter->second->set_online();
			++iter;
			continue;
		}

		PLAYER_PTR player = iter->second;
		if (player->need_kickout_room())
		{
			m_all_player.erase(iter++);
			if (-1 != player->get_seat_number())
			{
				m_desk->player_stand_up(player, GG_PlayerStandUpRequest::QUIT_ROOM);
			}
			//broadcast_player_quit(player, GG_PlayerQuitRoom_Request::USER_OFFLINE);
			destroy_player_i(player);
		}

		else
		{
			player->set_offline();
			++iter;
		}
	}
}


void GameRoom::broadcast_player_digcard(const PLAYER_PTR& player, const GG_PlayerDigCardRequest& request)
{
	GG_PlayerDigCardRequest sendreq = request;
	GG_PokerCard card = request.card();
	for (auto iter = m_all_player.begin(); iter != m_all_player.end(); ++iter)
	{
		if (iter->second->recv_broadcast())
		{
			if (iter->second != player)
			{
				sendreq.mutable_card()->set_color(0);
				sendreq.mutable_card()->set_point(0);
			}
			else
			{
				sendreq.mutable_card()->CopyFrom(card);
			}
			iter->second->send_request(GG_CMD_PLAYER_DIG_CARD, sendreq);
		}
	}
}



void GameRoom::broadcast_player_enter(const PLAYER_PTR& player)
{
	GG_PlayerEnterRoomRequest request;
	request.set_userid(player->get_user_id());
	broadcast_message(GG_CMD_PLAYER_ENTER_ROOM, request);
}