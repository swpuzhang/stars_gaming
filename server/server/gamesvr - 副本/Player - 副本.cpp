#include "Player.h"
#include "GameRoom.h"
#include "GG_GameMsg.pb.h"
#include "GG_GameCmd.pb.h"
#include "libserver/Session.h"
#include "GameDesk.h"
#include "GameSeat.h"

Player::Player() : m_online_check_time(time(NULL)), m_user_id(-1)
{

}

void Player::set_status(int status)
{
	if (status != m_cur_status)
	{
		INFO_LOG << "room(" << (m_room->get_room_info().m_room_id) << ") player(" << (m_user_id) << ":" 
			<< m_user_name << ") seat(" << (get_seat_number()) << ") status(" << get_status_name(m_cur_status) + " -> " + get_status_name(status) + ")";
		m_cur_status = status; //| (m_cur_status & ~PLAYER_MASK);
	}
}


std::string Player::get_status_name(int status) const
{

	std::string status_name;

	if (status & PLAYER_TRUSTEED)
	{
		status_name += "PLAYER_TRUSTEED|";
	}
	if (status & PLAYER_OFFLINE)
	{
		status_name += "PLAYER_OFFLINE|";
	}
	status &= PLAYER_MASK;

	switch (status)
	{
	case PLAYER_WAIT_NEXT_GAME:
	{
		status_name += "PLAYER_WAIT_NEXT_GAME";
		break;
	}
	case PLAYER_UNREADY:
	{
		status_name += "PLAYER_UNREADY";
		break;
	}
	case PLAYER_PLAYING:
	{
		status_name += "PLAYER_PLAYING";
		break;
	}
	case PLAYER_DROP:
	{
		status_name += "PLAYER_DROP";
		break;
	}
	case PLAYER_STAND_UP:
	{
		status_name += "PLAYER_STAND_UP";
		break;
	}
	default:
	{
		status_name += "UNKNOWN:" + toString(status);
		break;
	}
	}
	return status_name;
}

int Player::get_status(void) const
{
	return m_cur_status;
}

void Player::session_close()
{
	m_session.reset();
}

TY_INT64 Player::buy_in(TY_INT64 buy_count, TY_INT64 min_count)
{
	TRACE_FUNCATION();
	TY_INT64 real_buy_count = buy_chips(buy_count, min_count);
	if (real_buy_count < min_count)
	{
		INFO_LOG << "user(" << (m_user_id) << ") buy in failed";
		MAKE_PB(GG_PlayerBuyinRequest, buyin_request);
		buyin_request->set_buyresult(false);
		buyin_request->set_buyincount(buy_count);
		send_request(GG_CMD_PLAYER_BUYIN, buyin_request);
		return -1;
	}

	DEBUG_LOG << "palyer(" << (m_user_id) << ") buy_in carry_add:" << (real_buy_count);
	carry_add(real_buy_count);

	MAKE_PB(GG_PlayerBuyinRequest, buyin_request);
	buyin_request.set_buyresult(true);
	buyin_request.set_buyincount(real_buy_count);
	send_request(GG_CMD_PLAYER_BUYIN, buyin_request);
	return real_buy_count;
}


TY_INT64 Player::carry_add(TY_INT64 add_count)
{
	set_carry(m_cur_carry + add_count);
	DEBUG_LOG << "player(" << (m_user_id) << ") carry_add:"
		<< (add_count) << " cur_carry:" << (m_cur_carry);
	return m_cur_carry;
}

void Player::set_carry(TY_INT64 carry)
{
	DEBUG_LOG << "room(" << (m_room->get_room_info().m_room_id) << ") player(" << 
		(m_user_id) << ") carry(" << (m_cur_carry) << " -> " << (carry) << ")";
	m_cur_carry = carry;
}

bool Player::send_request(int cmd_type, const PBMESSAGE_PTR &msg)
{
	if (!m_session)
	{
		return false;
	}
	m_session->send_message(cmd_type, msg);
}

bool Player::send_request(const MESSAGE_PTR &msg)
{
	if (!m_session)
	{
		return false;
	}
	m_session->send_message(msg);
	return true;
}


TY_INT64 Player::buy_chips(TY_INT64 buy_count, TY_INT64 min_count)
{
	assert(buy_count >= min_count);
	MAKE_PB(GG_GameBuyChipsInRequest, request);
	request.set_userid(m_user_id);
	request.set_buychips(buy_count);
	request.set_buychipsmin(min_count);
	request.set_roomid(m_room->get_room_info().m_room_id);
	request.set_smallblind(m_room->get_room_info().m_small_blind);

	/*MESSAGE_PTR response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_GAME_BUY_CHIPS_IN, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send apply buyin failed");
		GG_UserLogRecordRequest log_req;
		log_req.set_logtime(time(NULL));
		log_req.set_userid(m_player_id);
		Json::Value log_info;
		log_info["action"] = "buy_chips";
		log_info["user_id"] = m_player_id;
		log_info["buy_count"] = toString(buy_count);
		log_info["min_count"] = toString(min_count);
		log_info["result"] = "send_moneysvr_failed";
		log_req.set_loginfo(json_to_string(log_info));
		MQService::instance()->send_mqrequest(GG_CMD_USERLOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), log_req);
		return -1;
	}

	if (!response.success() || response.m_result != GG_ERROR_SUCCESS)
	{
		LOG_ERROR("user(" + toString(m_player_id) + ")apply buyin failed(" + toString(response.m_result) + ")");
		GG_UserLogRecordRequest log_req;
		log_req.set_logtime(time(NULL));
		log_req.set_userid(m_player_id);
		Json::Value log_info;
		log_info["action"] = "buy_chips";
		log_info["user_id"] = m_player_id;
		log_info["buy_count"] = toString(buy_count);
		log_info["min_count"] = toString(min_count);
		log_info["result"] = "send_moneysvr_failed";
		log_req.set_loginfo(json_to_string(log_info));
		MQService::instance()->send_mqrequest(GG_CMD_USERLOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), log_req);
		return -1;
	}
	const GG_GameBuyChipsInResponse& pbresponse = response.ref_pbmsg<GG_GameBuyChipsInResponse>();
	m_active_chips = pbresponse.chipsleft();
	return pbresponse.buychipsreal();*/
	return m_room->get_room_info().m_small_blind;
}


void Player::set_trustee_off(void)
{
	if (is_player_trusteed_on())
	{
		set_status(m_cur_status & ~PLAYER_TRUSTEED);
		MAKE_PB(GG_PlayerTrusteeOffRequest, request)
		request->set_userid(get_user_id());
		m_room->broadcast_message(GG_CMD_PLAYER_TRUSTEE_OFF, request);
	}
}


bool Player::is_player_playing(void)
{
	int status = m_cur_status & PLAYER_MASK;
	if (status == PLAYER_PLAYING || status == PLAYER_DROP || status == PLAYER_DECLARE)
	{
		return true;
	}
	return false;
}


void Player::send_deal_cards(void)
{
	TRACE_FUNCATION();
	GG_DealCardsReqeust request;
	request.set_dealerseat(m_desk->get_dealer_index());
	request.set_dealcardcount(m_desk->get_start_deal_card_count());
	m_desk->get_player_deal_order(request.mutable_dealcardsorder());

	if (m_seat)
	{
		CopyCardToMsg(m_seat->get_hand_cards_total(), request.mutable_handcard());
	}
	request.set_bottomcardcount(m_desk->get_bottom_card().size());
	m_desk->get_pool_card().front().to_msg_card(request.mutable_firstpoolcard());
	m_desk->get_joker_card().to_msg_card(request.mutable_jokercard());
	send_request(GG_CMD_DEAL_CARDS, request);
}

bool Player::is_online(void)
{
	return PlayerManagerInstance::get_mutable_instance().is_player_online(m_user_id) && (m_online_check_time + 40 > time(NULL));
}


bool Player::is_player_offline()
{
	return m_cur_status & PLAYER_OFFLINE;
}

void Player::set_online()
{
	if (is_player_offline())
	{
		set_status(m_cur_status & ~PLAYER_OFFLINE);
	}
}

void Player::set_offline()
{
	if (!is_player_offline())
	{
		set_status(m_cur_status | PLAYER_OFFLINE);
	}
}

bool Player::need_kickout_room(void)
{
	return (m_online_check_time + 10 < time(NULL))
		&& (m_desk->is_game_over() || !is_seated_player());
}

void Player::send_player_stand_up(const PLAYER_PTR& player, int reason)
{
	TRACE_FUNCATION();
	GG_PlayerStandUpRequest pbrequest;
	pbrequest.set_userid(player->get_player_id());
	pbrequest.set_standupreason(reason);
	/*if (player->get_player_id() == m_player_id)
	{
	GG_Request msg;
	msg.m_cmd_type = GG_CMD_PLAYER_STANDUP;
	msg.set_pbmsg(&request);
	msg.session_id(m_session_id);
	m_pService->send_request(msg);
	}*/
	//else
	//{
	//if (player->get_player_id() == m_player_id)
	//{
	//	send_request_once(requst);
	//}
	//else
	//{
	send_request(GG_CMD_PLAYER_STANDUP, pbrequest);
	//}
	//}

}

int Player::get_seat_number(void) const
{
	return  m_seat ? m_seat->get_seat_number() : -1; 
}