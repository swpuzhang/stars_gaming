#include "GG_Player.h"
#include "GG_GameArea.h"
#include <assert.h>
#include "GG_Money_Proxy.h"
#include "GG_Game_Process.h"
#include "GG_Game_Process.h"
#include "GG_GameMsg.pb.h"
#include "GG_GameCmd.pb.h"
#include "GG_ErrorCode.pb.h"
#include "GG_RoomMsg.pb.h"
#include "GG_RoomCmd.pb.h"
#include "GG_SvrCmd.pb.h"
#include "GG_SvrMsg.pb.h"
#include "GG_HallMsg.pb.h"

using namespace GG_SvrMsg;
using namespace GG_SvrCmd;
using namespace GG_GameMsg;
using namespace GG_GameCmd;
using namespace GG_ErrorCode;
using namespace GG_RoomCmd;
using namespace GG_RoomMsg;
using namespace GG_HallMsg;



const int MAX_CHECK_ONLINE_TIMES = 5*60 / GG_GameArea::CHECK_ONLINE_INTERVAL;

GG_Player::GG_Player(void)
	: m_player_id(0), m_cur_status(PLAYER_UNREADY), m_online_check_time(time(NULL)), m_pGameArea(NULL)
	, m_cur_carry(0), m_pGameDesk(NULL), m_pGameSeat(NULL), m_is_recv_broadcast(false),  m_session_id(-1)
	,m_wait_send_queue(NULL), m_wait_resp_queue(NULL), m_retry_send_timerid(-1), m_pService(NULL)
    , m_rpc_mode(false), m_active_chips(0)
{
	m_platform_info["pid"] = 0;
	
}


GG_Player::~GG_Player(void)
{
    if (-1 != m_retry_send_timerid)
    {
        reactor()->cancel_timer(m_retry_send_timerid);
        m_retry_send_timerid = -1;
    }
    
    ACE_Message_Block *mb = NULL;
    ACE_Time_Value timeout(0, 0);
    while (-1 != m_wait_send_queue->dequeue(mb, &timeout))
    {
        delete (GG_Message*)mb->base();
        delete mb;
        mb = NULL;
    }
    while (-1 != m_wait_resp_queue->dequeue(mb, &timeout))
    {
        delete (GG_Message*)mb->base();
        delete mb;
        mb = NULL;
    }
    delete m_wait_resp_queue;
    delete m_wait_send_queue;
}


int GG_Player::handle_timeout (const ACE_Time_Value &current_time, const void *act)
{
    if (&m_retry_send_timerid == act)
    {
        m_retry_send_timerid = -1;
        retry_send_request_i();
        return 0;
    }
    return 0;
}


int GG_Player::send_request(int cmdType, PBMEssage &msg)
{
	GG_Request requst;
	requst.m_cmd_type = cmdType;
	requst.set_pbmsg(&msg);
	return send_request(requst);
}

int GG_Player::send_request(const GG_Message &msg)
{
	// 如果不采用RPC 方式，则直接发送
	if (!m_rpc_mode)
	{
		msg.session_id(m_session_id);
		return m_pService->send_request(msg);
	}

	// 采用RPC 方式，入队后发送
	GG_Message *request = msg.clone();
	ACE_Message_Block *mb = new ACE_Message_Block((const char*)request);
	ACE_Time_Value timeout(0, 0);
	if (-1 == m_wait_send_queue->enqueue(mb, &timeout))
	{
		LOG_ERROR("put send_q failed. count(" + toString(m_wait_send_queue->message_count()) + ")");
		delete request;
		delete mb;
		return -1;
	}

    // 如果没有等待确认的消息，马上发送
    if (m_wait_resp_queue->message_count() == 0)
    {
        send_request_i();
    }
    return 0;
}

// 一次性立即投递消息，不重传
int GG_Player::send_request_once(const GG_Message &msg)
{
    // 则直接发送
    msg.session_id(m_session_id);
    return m_pService->send_request(msg);
}

int GG_Player::send_request_i(void)
{
	if (0 == m_wait_send_queue->message_count())
	{
		return 0;
	}

	ACE_Message_Block *mb = NULL;
	ACE_Time_Value timeout(0, 0);
	if (-1 == m_wait_send_queue->dequeue(mb, &timeout))
	{
		LOG_ERROR("pop send_q failed. count(" + toString(m_wait_send_queue->message_count()) + ")");
		return -1;
	}

	GG_Message *request = (GG_Message*)mb->base();
	timeout.set(0, 0);
	if (-1 == m_wait_resp_queue->enqueue(mb, &timeout))
	{
		LOG_ERROR("put send_q failed. count(" + toString(m_wait_send_queue->message_count()) + ")");
		delete request;
		delete mb;
		return -1;
	}

	request->session_id(m_session_id);

	if (-1 == m_pService->send_request(*request, on_user_response, (void*)(long)m_player_id, ACE_Time_Value(5,0)))
	{
		LOG_ERROR("send user(" + toString(m_player_id) + ") request failed. timer to send_request_retry");
		if (-1 == m_retry_send_timerid)
		{
			m_retry_send_timerid = reactor()->schedule_timer(this, &m_retry_send_timerid, ACE_Time_Value(1));
		}

		if (-1 == m_retry_send_timerid)
		{
			LOG_ERROR("send user(" + toString(m_player_id) + ") request failed. register retry send timer failed");
		}
		return 0;
	}

	return 0;
}

int GG_Player::retry_send_request_i(void)
{
    LOG_DEBUG("user(" + toString(m_player_id) + ") retry send. wait_send(" + toString(m_wait_send_queue->message_count()) + ") wait_resp(" + toString(m_wait_resp_queue->message_count()) + ")");
    ACE_Message_Block *mb = NULL;
    ACE_Time_Value timeout(0, 0);
    if (-1 == m_wait_resp_queue->peek_dequeue_head(mb, &timeout))
    {
        LOG_ERROR("peek send_q failed. count(" + toString(m_wait_send_queue->message_count()) + ")");
        return -1;
    }
    GG_Message *request = (GG_Message*)mb->base();
    request->session_id(m_session_id);

    if (-1 == m_pService->send_request_retry(*request, on_user_response, (void*)(long)m_player_id, ACE_Time_Value(5,0)))
    {
        LOG_ERROR("send user(" + toString(m_player_id) + ") request failed. timer to send_request_retry");
        if (-1 == m_retry_send_timerid)
        {
            m_retry_send_timerid = reactor()->schedule_timer(this, &m_retry_send_timerid, ACE_Time_Value(1));
        }

        if (-1 == m_retry_send_timerid)
        {
            LOG_ERROR("send user(" + toString(m_player_id) + ") request failed. register retry send timer failed");
        }
        return 0;
    }
    return 0;
}


void GG_Player::set_player_id(int id)
{
	m_player_id = id;
}

int GG_Player::get_player_id(void)
{
	return m_player_id;
}

void GG_Player::set_session_id(long session_id)
{
    LOG_INFO("user(" + toString(m_player_id) + ") session(" + toString(m_session_id) + " -> " + toString(session_id) + ")");
    m_session_id = session_id;
}

long GG_Player::get_session_id(void)
{
    return m_session_id;
}

void GG_Player::set_player_name(const std::string& name)
{
	m_player_name = name;
}

const std::string& GG_Player::get_player_name(void)
{
	return m_player_name;
}

void GG_Player::set_status(int status)
{
	if (status != m_cur_status)
	{
		LOG_INFO("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") player(" + toString(m_player_id) + ":" + m_player_name + ") seat(" + toString(get_seat_number()) + ") status(" + get_status_name(m_cur_status) + " -> " + get_status_name(status) + ")");
		m_cur_status = status; //| (m_cur_status & ~PLAYER_MASK);
	}
}

int GG_Player::get_status(void)
{
	return m_cur_status;
}

void GG_Player::flush_online_time(void)
{
	TRACE_FUNCATION();
	m_online_check_time = ::time(NULL);
	set_online();
}

bool GG_Player::is_online(void)
{
	return GamePlayer_Manager::instance()->is_player_online(m_player_id) && (m_online_check_time + 40 > time(NULL));
}

bool GG_Player::need_kickout_room(void)
{
	return (m_online_check_time + 10 < time(NULL)) 
		&& (m_pGameDesk->is_game_over() || !is_seated_player());
}

GG_INT64 GG_Player::get_carry(void)
{
	return m_cur_carry;
}

void GG_Player::set_carry(GG_INT64 carry)
{
	LOG_DEBUG("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") player(" + toString(m_player_id) + ") carry(" + toString(m_cur_carry) + " -> " + toString(carry) + ")");
	m_cur_carry = carry;
}

GG_INT64 GG_Player::carry_add(GG_INT64 add_count)
{
	set_carry(m_cur_carry + add_count);
	LOG_DEBUG("player(" + toString(m_player_id) + ") carry_add:" 
		+ toString(add_count) + " cur_carry:" + toString(m_cur_carry));
	return m_cur_carry;
}

GG_INT64 GG_Player::carry_sub(GG_INT64 sub_count)
{
	set_carry(m_cur_carry - sub_count);
	LOG_DEBUG("player(" + toString(m_player_id) + ") carry_sub:" 
		+ toString(sub_count) + " cur_carry:" + toString(m_cur_carry));
	return m_cur_carry;
}

void GG_Player::send_deal_cards(void)
{
    TRACE_FUNCATION();
    GG_DealCardsReqeust request;
	request.set_dealerseat(m_pGameDesk->get_dealer_index());
	request.set_dealcardcount(m_pGameDesk->get_start_deal_card_count());
	m_pGameDesk->get_player_deal_order(request.mutable_dealcardsorder());
	
    if (m_pGameSeat)
    {
		CopyCardToMsg(m_pGameSeat->get_hand_cards_total(), request.mutable_handcard());
    }
	request.set_bottomcardcount(m_pGameDesk->get_bottom_card().size());
	m_pGameDesk->get_pool_card().front().to_msg_card(request.mutable_firstpoolcard());
	m_pGameDesk->get_joker_card().to_msg_card(request.mutable_jokercard());
    send_request(GG_CMD_DEAL_CARDS, request);
}

void GG_Player::set_game_desk(GG_GameDesk *desk)
{
	m_pGameDesk = desk;
}

GG_INT64 GG_Player::buy_chips(GG_INT64 buy_count, GG_INT64 min_count)
{
	assert(buy_count >= min_count);
	GG_GameBuyChipsInRequest request;
	request.set_userid(m_player_id);
	request.set_buychips(buy_count);
	request.set_buychipsmin(min_count);
	request.set_roomid(m_pGameArea->area_id());
	request.set_smallblind(m_pGameArea->get_area_info().m_small_blind);

	GG_Response response;
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
	return pbresponse.buychipsreal();
}

// 座位上购买筹码增加携带
GG_INT64 GG_Player::buy_add_carry_chips(GG_INT64 need_add_carry)
{
	GG_GameAddCarryRequest request;

	request.set_userid(m_player_id);
	request.set_roomid(m_pGameArea->area_id());
	request.set_curcarrychips(m_cur_carry);
	request.set_needaddtocarry(need_add_carry);

	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_GAME_ADD_CARRY, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send apply buyin failed");
		GG_UserLogRecordRequest log_req;
		log_req.set_logtime(time(NULL));
		log_req.set_userid(m_player_id);
		Json::Value log_info;
		log_info["action"] = "buy_chips";
		log_info["user_id"] = m_player_id;
		log_info["buy_count"] = toString(need_add_carry);
		log_info["result"] = "send_moneysvr_failed";
		log_req.set_loginfo(json_to_string(log_info));
		MQService::instance()->send_mqrequest(GG_CMD_USERLOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), log_req);
		return -1;
	}

	if (!response.success() || response.m_result != GG_ERROR_SUCCESS)
	{
		LOG_ERROR("player(" + toString(m_player_id) + ")apply buyin failed(" + toString(response.m_result) + ")");
		GG_UserLogRecordRequest log_req;
		log_req.set_logtime(time(NULL));
		log_req.set_userid(m_player_id);
		Json::Value log_info;
		log_info["action"] = "buy_chips";
		log_info["user_id"] = m_player_id;
		log_info["buy_count"] = toString(need_add_carry);
		log_info["result"] = "send_moneysvr_failed";
		log_req.set_loginfo(json_to_string(log_info));
		MQService::instance()->send_mqrequest(GG_CMD_USERLOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), log_req);

		return -1;
	}
	GG_GameAddCarryResponse pbresponse;
	if (m_active_chips < need_add_carry)
	{
		return -1;
	}
	m_active_chips -= need_add_carry;

	return need_add_carry;
}

GG_INT64 GG_Player::buy_in(GG_INT64 buy_count, GG_INT64 min_count)
{
	TRACE_FUNCATION();
	GG_INT64 real_buy_count = buy_chips(buy_count, min_count);
	if (real_buy_count < min_count)
	{
		LOG_INFO("user(" + toString(m_player_id) + ") buy in failed");
		GG_PlayerBuyinRequest buyin_request;
		buyin_request.set_buyresult(false);
		buyin_request.set_buyincount(buy_count);
		send_request(GG_CMD_PLAYER_BUYIN, buyin_request);
		return -1;
	}
	LOG_DEBUG("palyer(" + toString(m_player_id) + ") buy_in carry_add:"
		+ toString(real_buy_count));
	carry_add(real_buy_count);

	GG_PlayerBuyinRequest buyin_request;

	buyin_request.set_buyresult(true);
	buyin_request.set_buyincount(real_buy_count);

	send_request(GG_CMD_PLAYER_BUYIN, buyin_request);
	return real_buy_count;
}

void GG_Player::set_game_seat(GG_GameSeat *seat)
{
	m_pGameSeat = seat;
}

bool GG_Player::recv_broadcast(void)
{
	return m_is_recv_broadcast;
}

void GG_Player::recv_broadcast(bool is_recv)
{
	m_is_recv_broadcast = is_recv;
}

std::string GG_Player::get_status_name(int status) const
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


int  GG_Player::get_seat_number(void)
{
	return m_pGameSeat ? m_pGameSeat->get_seat_number() : -1;
}


void GG_Player::stand_up(void)
{
	TRACE_FUNCATION();
	assert(m_pGameSeat != NULL);
	this->set_game_seat(NULL);
	this->set_status(PLAYER_STAND_UP | (m_cur_status & ~PLAYER_MASK));
	GG_UserLeaveGameRequest request;
	request.set_userid(m_player_id);
	request.set_roomid(m_pGameArea->area_id());
	request.set_playingcount(m_pGameDesk->get_seated_count());
	request.set_givebackchips(m_cur_carry);
	MQService::instance()->send_mqrequest(GG_CMD_USER_LEAVE_GAME, MQ_TRANSMIT_TOPIC,
		SITE_NAME_ROOMSVR + "." + Game_Process::instance()->getGameName(), request);

	MQService::instance()->send_mqrequest(GG_CMD_USER_LEAVE_GAME, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, request);
	send_game_leave_account();
	
	if (m_cur_carry > 0)
	{
		LOG_DEBUG("palyer(" + toString(m_player_id) + ") stand up carry_sub:"
			+ toString(m_cur_carry));
		carry_sub(m_cur_carry);
	}
} 

void GG_Player::sit_down(GG_GameSeat *seat)
{
	this->set_game_seat(seat);
	GG_UserJoinGameRequest request;
	request.set_userid(m_player_id);
	request.set_roomid(m_pGameArea->get_area_info().m_area_id);
	request.set_smallblind(m_pGameArea->get_area_info().m_small_blind);
	request.set_playingcount(m_pGameDesk->get_seated_count());
	DEBUG_LOG << "room(" << m_pGameArea->get_area_info().m_area_id << ") send playing_count(" << request.playingcount() << ")";
	MQService::instance()->send_mqrequest(GG_CMD_USER_JOIN_GAME, MQ_TRANSMIT_TOPIC,
		SITE_NAME_ROOMSVR + "." + Game_Process::instance()->getGameName(), request);
	MQService::instance()->send_mqrequest(GG_CMD_USER_JOIN_GAME, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, request);
}

int GG_Player::present_chips_to_other(GG_INT64 chips_count, GG_Player *other)
{
	//if (m_cur_carry < chips_count)
	//{
	//	LOG_ERROR("present chips(" + toString(chips_count) + ") big then carry(" + toString(m_cur_carry) + ")");
	//	return -1;
	//}
	//this->carry_sub(chips_count);
	//other->carry_add(chips_count);

	//GG_PlayerPresentChips_Request request;
	//request.m_cmd_type = GG_CMD_PLAYER_PRESENT_CHIPS;
	//request.m_presenter_id = m_player_id;
	//request.m_acceptor_id = other->m_player_id;
	//request.m_chips_count = chips_count;
	//request.m_presenter_carry = m_cur_carry;
	//request.m_acceptor_carry = other->get_carry();
	//m_pGameArea->broadcast_message(request);
	//m_pGameDesk->get_game_log_record()->player_present_chips(request);
	return 0;
}

GG_GameSeat* GG_Player::get_seat(void)
{
	return m_pGameSeat;
}

bool GG_Player::is_seated_player(void)
{
	return NULL != m_pGameSeat;
}

GG_GameArea* GG_Player::get_game_area(void)
{
	return m_pGameArea;
}

void GG_Player::set_platform(const Json::Value& platform)
{
	m_platform_info = platform;
}

const Json::Value& GG_Player::get_platform(void)
{
	return m_platform_info;
}

void GG_Player::send_game_leave_account(const std::string& game_id)
{
	TRACE_FUNCATION();
	GG_GameLeaveAccountRequest request;
	request.set_userid(m_player_id);
	request.set_currentcarry(m_cur_carry);
	request.set_gameid(game_id);
	request.set_roomid(m_pGameArea->get_area_info().m_area_id);

	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_GAME_LEAVE_ACCOUNT, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, on_user_leave_game_response, NULL, ACE_Time_Value(5, 0)))
	{
		LOG_ERROR("send leave game failed");
		GG_UserLogRecordRequest log_req;
		log_req.set_logtime(time(NULL));
		log_req.set_userid(m_player_id);
		Json::Value log_info;
		log_info["action"] = "game_leave_account";
		log_info["user_id"] = m_player_id;
		log_info["add_count"] = toString(m_cur_carry);
		log_info["add_reason"] = GG_PlayerChipsChangeRequest::game_give_back;
		log_info["result"] = "send_moneysvr_failed";
		log_req.set_loginfo(json_to_string(log_info));
		MQService::instance()->send_mqrequest(GG_CMD_USERLOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), log_req);
	}
}


void GG_Player::ready_for_game(void)
{
    set_status((m_cur_status & ~PLAYER_MASK) | PLAYER_PLAYING);
}


bool GG_Player::check_chips_carry(void)
{
    LOG_DEBUG("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") player(" + toString(m_player_id) + ") cur_carry(" + toString(m_cur_carry) + ") small_blind(" + toString(m_pGameArea->get_area_info().m_small_blind) + ")");
    GG_INT64 need_add_carry = m_pGameDesk->get_sitdown_buyin_count() - m_cur_carry;
    if (need_add_carry <= 0)
    {
        return true;
    }

    GG_INT64 real_buy_count = buy_add_carry_chips(need_add_carry);
    if (real_buy_count < need_add_carry)
    {
		LOG_INFO("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") player(" + toString(m_player_id) + ") buy_add_carry_chips failed");
		GG_PlayerBuyinRequest buyin_request;

		buyin_request.set_buyincount(need_add_carry);
		buyin_request.set_buyresult(false);
		send_request(GG_CMD_PLAYER_BUYIN, buyin_request);

        return false;
    }

	LOG_DEBUG("palyer(" + toString(m_player_id) + ") check_chips_carry carry_add:"
		+ toString(real_buy_count));
    carry_add(real_buy_count);

	GG_PlayerBuyinRequest buyin_request;
	buyin_request.set_buyresult(true);
	buyin_request.set_buyincount(real_buy_count);
	send_request(GG_CMD_PLAYER_BUYIN, buyin_request);
    return true;
}

void GG_Player::send_user_box_time_recover(void)
{
   
}

void GG_Player::send_user_box_time_pause(void)
{
   
}

GG_INT64 GG_Player::get_user_money()
{
	Json::Value money_info;
	if (-1 == MoneyProxy::instance()->get_user_money(m_player_id, money_info))
	{
		LOG_DEBUG("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") player(" + toString(m_player_id) + ") get_user_money failed");
		GG_UserLogRecordRequest log_req;
		log_req.set_logtime(time(NULL));
		log_req.set_userid(m_player_id);
		Json::Value log_info;
		log_info["action"] = "get_user_money";
		log_info["user_id"] = m_player_id;
		log_info["add_count"] = toString(m_cur_carry);
		log_info["result"] = "send_moneysvr_failed";
		log_req.set_loginfo(json_to_string(log_info));
		MQService::instance()->send_mqrequest(GG_CMD_USERLOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), log_req);

		return 0;
	}

	if (!money_info["chips"].isNull() && money_info["chips"].isString())
	{
		m_active_chips = BaseTool::atoi64(money_info["chips"].asString());
	}
	if (!money_info["carry"].isNull() && money_info["carry"].isString())
	{
		m_cur_carry = BaseTool::atoi64(money_info["carry"].asString());
	}

	return m_active_chips + m_cur_carry;
}

GG_INT64 GG_Player::get_chips_active(void)
{
	Json::Value money_info;
	if (-1 == MoneyProxy::instance()->get_user_money(m_player_id, money_info))
	{
		LOG_DEBUG("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") player(" + toString(m_player_id) + ") get_user_money failed");

		return 0;
	}

	return money_info["chips"].isString() ? BaseTool::atoi64(money_info["chips"].asString()) : 0;
}


int GG_Player::add_active_money(const GG_INT64 carray)
{
	
	return 0;
}

void GG_Player::deal_loser_account(const GG_INT64 all_loser_count, GG_INT64& real_count, 
								   GG_INT64& all_chips, int room_id, const std::string& game_id)
{
	Json::Value money_info;
	if (-1 == MoneyProxy::instance()->deal_lose_account(m_player_id, m_cur_carry,
		m_pGameArea->get_game_desk()->get_sitdown_buyin_count(), all_loser_count, room_id, game_id, money_info))
	{
		LOG_DEBUG("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") player(" + toString(m_player_id) + ") get_user_money failed");

		GG_UserLogRecordRequest log_req;
		log_req.set_logtime(time(NULL));
		log_req.set_userid(m_player_id);
		Json::Value log_info;
		log_info["action"] = "deal_loser_account";
		log_info["user_id"] = m_player_id;
		log_info["all_loser_count"] = toString(all_loser_count);
		log_info["result"] = "deal_loser_account_failed";
		log_req.set_loginfo(json_to_string(log_info));
		MQService::instance()->send_mqrequest(GG_CMD_USERLOG_RECORD, MQ_TRANSMIT_DIRECT, SITE_NAME_GAMELOGSVR + "." + Game_Process::instance()->getGameName(), log_req);
		return;
	}

	m_cur_carry = BaseTool::atoi64(money_info["carry"].asString());
	m_active_chips = BaseTool::atoi64(money_info["chips"].asString());
	real_count = BaseTool::atoi64(money_info["real_account"].asString());
	LOG_INFO("real_count:" + money_info["real_account"].asString());
	all_chips = m_cur_carry + m_active_chips;
}

void GG_Player::set_offline() 
{
	if (!is_player_offline())
	{
		set_status(m_cur_status | PLAYER_OFFLINE);
	}
}
void GG_Player::set_online() 
{
	if (is_player_offline())
	{
		set_status(m_cur_status & ~PLAYER_OFFLINE);
	}
}

bool GG_Player::is_player_offline() 
{ 
	return m_cur_status & PLAYER_OFFLINE;
}

int GG_Player::on_recv_response(const GG_Message &request, const GG_Response &response)
{
	if (!response.success())
	{
		LOG_WARN("user(" + toString(m_player_id) + ") resp failed. seq(" + toString(request.sequence_id()) + ")");
		request.session_id(m_session_id);

		ACE_Message_Block *mb = NULL;
		ACE_Time_Value timeout(0, 0);
		if (-1 == m_wait_resp_queue->peek_dequeue_head(mb, &timeout))
		{
			LOG_ERROR("peek wait_resp failed. count(" + toString(m_wait_send_queue->message_count()) + "), no need send retry");
			return -1;
		}
		GG_Message *head_req = (GG_Message*)mb->base();
		if (head_req->sequence_id() != request.sequence_id())
		{
			LOG_ERROR("first wait resp request seq(" + toString(head_req->sequence_id()) + ") not the response seq(" + toString(request.sequence_id()) + "), no need send retry");
			return -1;
		}

		LOG_DEBUG("room(" + toString(m_pGameArea->get_area_info().m_area_id) + ") user(" + toString(m_player_id) + ") send_retry seq(" + toString(request.sequence_id()) + ")");
		if (-1 == m_pService->send_request_retry(request, on_user_response, (void*)(long)m_player_id, ACE_Time_Value(5,0)))
		{
			LOG_ERROR("send user(" + toString(m_player_id) + ") request failed. timer to send_request_retry");
			if (-1 == m_retry_send_timerid)
			{
				m_retry_send_timerid = reactor()->schedule_timer(this, &m_retry_send_timerid, ACE_Time_Value(1));
			}

			if (-1 == m_retry_send_timerid)
			{
				LOG_ERROR("send user(" + toString(m_player_id) + ") request failed. register retry send timer failed");
			}
			return 0;
		}
		return -1;
	}

	flush_online_time();

	// 不管应用层的错误码，只确认消息有回应
	ACE_Message_Block *mb = NULL;
	ACE_Time_Value timeout(0, 0);
	if (-1 == m_wait_resp_queue->peek_dequeue_head(mb, &timeout))
	{
		LOG_ERROR("peek wait_resp failed. count(" + toString(m_wait_send_queue->message_count()) + ")");
		return -1;
	}
	GG_Message *head_req = (GG_Message*)mb->base();
	if (head_req->sequence_id() != response.sequence_id())
	{
		LOG_ERROR("first wait resp request seq(" + toString(head_req->sequence_id()) + ") not the response seq(" + toString(response.sequence_id()) + ")");
		return -1;
	}

	LOG_DEBUG("first wait resp request seq(" + toString(head_req->sequence_id()) + ") recv the response");
	if (-1 == m_wait_resp_queue->dequeue_head(mb, &timeout))
	{
		assert(0);
		LOG_ERROR("peek wait_resp failed. count(" + toString(m_wait_send_queue->message_count()) + ")");
		return -1;
	}
	delete (GG_Message*)mb->base();
	delete mb;

	send_request_i();
	return 0;
}

bool GG_Player::is_in_room(void)
{
	return NULL != m_pGameArea;
}


void GG_Player::send_player_stand_up(GG_Player *player, int reason)
{
	TRACE_FUNCATION();
	GG_PlayerStandUpRequest pbrequest;
	pbrequest.set_userid(player->get_player_id());
	pbrequest.set_standupreason(reason);
	GG_Request requst;
	requst.m_cmd_type = GG_CMD_PLAYER_STANDUP;
	requst.set_pbmsg(&pbrequest);
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
	if (player->get_player_id() == m_player_id)
	{
		send_request_once(requst);
	}
	else
	{
		send_request(requst);
	}
	//}
	
}

bool GG_Player::need_set_offline(void)
{
	return m_online_check_time + 3 < time(NULL);
}



bool GG_Player::is_player_need_standup()
{
	if (is_player_offline() || is_player_trusteed_on() || is_player_escaped())
	{
		return true;
	}
	return false;
}

void GG_Player::delete_player_message()
{
	if (-1 != m_retry_send_timerid)
	{
		reactor()->cancel_timer(m_retry_send_timerid);
		m_retry_send_timerid = -1;
	}

	ACE_Message_Block *mb = NULL;
	ACE_Time_Value timeout(0, 0);
	while (-1 != m_wait_send_queue->dequeue(mb, &timeout))
	{
		delete (GG_Message*)mb->base();
		delete mb;
		mb = NULL;
	}
	while (-1 != m_wait_resp_queue->dequeue(mb, &timeout))
	{
		delete (GG_Message*)mb->base();
		delete mb;
		mb = NULL;
	}
}


void GG_Player::set_trustee_on(void)
{
	if (!is_player_trusteed_on())
	{
		LOG_DEBUG("player(" + toString(m_player_id) + ") trustee status (START_TRUSTEE)");
		set_status(m_cur_status | PLAYER_TRUSTEED);

		// 通知客户端
		GG_PlayerTrusteeOnRequest request;
		request.set_userid(get_player_id());
		m_pGameArea->broadcast_message(GG_CMD_PLAYER_TRUSTEE_ON, request);
	}
	
}

void GG_Player::set_trustee_off(void)
{
	if (is_player_trusteed_on())
	{
		LOG_DEBUG("player(" + toString(m_player_id) + ") trustee status (END_TRUSTEE)");
		set_status(m_cur_status & ~PLAYER_TRUSTEED);

		// 通知客户端
		GG_PlayerTrusteeOffRequest request;
		request.set_userid(get_player_id());
		m_pGameArea->broadcast_message(GG_CMD_PLAYER_TRUSTEE_OFF, request);
	}
}

bool GG_Player::is_player_trusteed_on(void)
{
	return m_cur_status & PLAYER_TRUSTEED;
}

void GG_Player::set_escaped() 
{
	if (!is_player_escaped())
	{
		set_status(m_cur_status | PLAYER_ESCAPED);
		set_offline();
		set_trustee_on();
	}
}
void GG_Player::set_unescaped() 
{
	if (is_player_escaped())
	{
		set_status(m_cur_status & ~PLAYER_ESCAPED);
		set_online();
		set_trustee_off();
	}
}

void GG_Player::come_back_game(void)
{
	if (is_player_escaped())
	{
		set_unescaped();
		m_pGameDesk->get_game_log_record()->come_back_game(m_player_id, get_seat_number());
	}
}

bool GG_Player::is_player_escaped() 
{ 
	return m_cur_status & PLAYER_ESCAPED;
}

void GG_Player::escape_game(void)
{
	set_escaped();

	m_pGameDesk->get_game_log_record()->player_escape(m_player_id, get_seat_number());

	GG_UserEscapeGameRequest request;
	request.set_userid(m_player_id);
	request.set_roomid(m_pGameArea->area_id());
	request.set_smallblind(m_pGameArea->get_area_info().m_small_blind);
	request.set_playingcount(m_pGameDesk->get_seated_count());
	DEBUG_LOG << "room(" << m_pGameArea->get_area_info().m_area_id << ") send playing_count(" << request.playingcount() << ")";
	MQService::instance()->send_mqrequest(GG_CMD_USER_ESCAPE_GAME, MQ_TRANSMIT_TOPIC,
		SITE_NAME_ROOMSVR + "." + Game_Process::instance()->getGameName(), request);
}

int GG_Player::add_chips(GG_INT64 chips, int add_reason)
{
	Json::Value jv;
	if (-1 == MoneyProxy::instance()->add_user_chips(m_player_id, chips, add_reason, jv))
	{
		LOG_ERROR("user(" + toString(m_player_id) + ") add_chips(" + toString(chips) + ") failed");
		return -1;
	}
	m_active_chips = BaseTool::atoi64(jv[RPC_PARAMETER]["chips"].asString());
	m_cur_carry = BaseTool::atoi64(jv[RPC_PARAMETER]["carry"].asString());
	return 0;
}

int GG_Player::add_carry(GG_INT64 chips, int add_reason)
{
	Json::Value jv;
	if (-1 == MoneyProxy::instance()->add_user_carry(m_player_id, chips, add_reason, jv))
	{
		LOG_ERROR("user(" + toString(m_player_id) + ") add_chips(" + toString(chips) + ") failed");
		return -1;
	}
	m_active_chips = BaseTool::atoi64(jv[RPC_PARAMETER]["chips"].asString());
	m_cur_carry = BaseTool::atoi64(jv[RPC_PARAMETER]["carry"].asString());
	return 0;
}

int GG_Player::bet_chips(GG_INT64 chips, int add_reason, GG_INT64& real_bet)
{
	Json::Value jv;
	if (-1 == MoneyProxy::instance()->bet_chips(m_player_id, chips, add_reason, jv))
	{
		LOG_ERROR("user(" + toString(m_player_id) + ") add_chips(" + toString(chips) + ") failed");
		return -1;
	}
	m_active_chips = BaseTool::atoi64(jv[RPC_PARAMETER]["chips"].asString());
	m_cur_carry = BaseTool::atoi64(jv[RPC_PARAMETER]["carry"].asString());
	real_bet = BaseTool::atoi64(jv[RPC_PARAMETER]["real_bet"].asString());
	return 0;
}


void GG_Player::send_player_quit_room(GG_Player *player, GG_UINT8 reason)
{
	GG_PlayerQuitRoomRequest request;
	request.set_userid(player->get_player_id());
	request.set_quitreason( reason);
	send_request(GG_CMD_PLAYER_QUIT_ROOM, request);
}

bool GG_Player::is_player_playing(void)
{
	int status = m_cur_status & PLAYER_MASK;
	if (status == PLAYER_PLAYING || status == PLAYER_DROP || status == PLAYER_DECLARE)
	{
		return true;
	}
	return false;
}