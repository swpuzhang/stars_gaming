#include "GameDesk.h"
#include "RoomManager.h"
#include "Player.h"
#include "GameRoom.h"
#include "CardDealer.h"
#include "Combination.h"
#include <assert.h>
#include <fstream>
#include <algorithm>

#include "GG_GameMsg.pb.h"
#include "GG_GameCmd.pb.h"
#include "GG_ErrorCode.pb.h"
#include "GG_RoomMsg.pb.h"
#include "GG_RoomCmd.pb.h"
#include "GG_SvrCmd.pb.h"
#include "GG_SvrMsg.pb.h"
#include "GG_SystemCmd.pb.h"
#include "GG_SystemMsg.pb.h"
#include "GG_HallMsg.pb.h"


using namespace GG_SystemMsg;
using namespace GG_SystemCmd;
using namespace GG_SvrMsg;
using namespace GG_SvrCmd;
using namespace GG_GameMsg;
using namespace GG_GameCmd;
using namespace GG_ErrorCode;
using namespace GG_RoomCmd;
using namespace GG_RoomMsg;
using namespace GG_HallMsg;

static void get_card_from_file(std::vector<PokerCard> &vec_card, std::vector<PokerCard> &all_card,
	int& draw_sec, int& left_bottom_count, TY_INT64 small_blind);

GameDesk::GameDesk(const GAMEROOM_PTR& room)
	: m_max_seat_num(0), m_dealer_index(0), m_game_status(GAME_UNREADY)
	, m_room(room),  m_wait_round_timer(RoomManagerInstance::get_mutable_instance().get_io_loop())
	, m_notify_status_timer(RoomManagerInstance::get_mutable_instance().get_io_loop()), m_min_game_player_count(4), m_apply_declare_seatnum(-1)
	,m_notify_room_timerid(-1), m_last_winner_seatnum(-1), m_chips_pool(0), m_wait_begin(0), m_time_game_begin(-1)
	,m_active_seatnum(-1), m_gameover_type(-1),  m_notify_room_type(0), m_wait_game_sec(0), m_draw_sec(0)

{
	m_max_seat_num = m_room->get_room_info().m_max_playing;
	if (5 != m_max_seat_num)
	{
		LOG_ERROR("max seat num error");
		m_max_seat_num = 5;
	}
	m_min_game_player_count = 2;
	init_seats();
	m_game_log_record.set_room_info(m_room->get_room_info());
}


GameDesk::~GameDesk(void)
{

}

void GameDesk::init_seats(void)
{
	m_seats.resize(m_max_seat_num);
	for (int i = 0; i < m_max_seat_num; i++)
	{
		m_seats[i].set_game_desk(shared_from_this());
		m_seats[i].set_seat_number(i);
	}
}

void GameDesk::notify_room_status(int type)
{
	GG_NotifyRoomStausRequest request;
	request.set_svrport(GameArea_Manager::instance()->get_gamesvr_port());
	request.mutable_info()->set_roomid(m_room->get_room_info().m_area_id);
	request.mutable_info()->set_playingcount(get_seated_count());
	request.mutable_info()->set_status(type);
	//MQService::instance()->send_mqrequest(GG_CMD_NOTIFY_ROOM_STATUS, MQ_TRANSMIT_TOPIC, SITE_NAME_ROOMSVR + "." + Game_Process::instance()->getGameName()
	//	, request);
	m_notify_room_type = type;
}

void GameDesk::on_wait_round_timeout(const SYSTEM_CODE& err)
{
	TRACE_FUNCATION();
	if (!err)
	{	
		step_new_round();
		/*else if (act == &m_notify_room_timerid)
		{
			m_notify_room_timerid = -1;
			notify_room_status(GG_FlushRoomStausRequest::ROOM_PLAYING);
		}*/
	}
}


bool GameDesk::is_game_wait_over(void)
{
	return (m_game_status & GAME_WAIT_MASK);
}

int GameDesk::player_sit_down(const PLAYER_PTR& player, int seat_number)
{
	TRACE_FUNCATION();
	if (seat_number < 0 || seat_number > (m_max_seat_num - 1))
	{
		return -1;
	}

	if (m_seats[seat_number].is_seated())
	{
		return -1;
	}
	m_seats[seat_number].player_sit_down(player);

	// 清除玩家之前的托管状态
	player->set_trustee_off();

	//如果GameOver 该玩家可以加入游戏
	if (!is_game_over())
	{
		LOG_DEBUG("game is not over status:" + get_status_name(m_game_status));
		
		player->set_status((player->get_status() & ~Player::PLAYER_MASK) | Player::PLAYER_WAIT_NEXT_GAME);
	}
	else
	{
		LOG_DEBUG("game is over status:" + get_status_name(m_game_status));
		player->set_status((player->get_status() & ~Player::PLAYER_MASK) | Player::PLAYER_UNREADY);
		if (m_game_status & GAME_UNREADY)
		{
			wait_for_round(GAME_UNREADY, 0);
		}
	}
	
	m_room->broadcast_player_seat(seat_number, *player);
	return seat_number;
}

bool GameDesk::is_seat_idle(int seat_number)
{
	if (seat_number < 0 || seat_number > (m_max_seat_num - 1))
	{
		return false;
	}

	return !m_seats[seat_number].is_seated();
}


void GameDesk::get_joingame_player(google::protobuf::RepeatedPtrField<PB_IngamePlayer>* player_seat)
{
	TRACE_FUNCATION();
	if (is_game_over())
	{
		return;
	}
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (!iter->is_player_joined())
		{
			continue;
		}

		PB_IngamePlayer* player = player_seat->Add();
		player->set_userid(iter->get_ingame_player_id());
		player->set_seatnum(iter->get_seat_number());
		DEBUG_LOG << "get_joingame_player userid(" << player->userid() << ") seat_num(" << player->seatnum() << ")";
	}
}

void GameDesk::get_seat_player(google::protobuf::RepeatedPtrField<GG_PlayerInfo>* player_seat)
{
	TRACE_FUNCATION();
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (!iter->is_seated())
		{
			continue;
		}
       
		GG_PlayerInfo *player = player_seat->Add();
		player->set_id((*iter).get_seated_player()->get_user_id());
		player->set_name((*iter).get_seated_player()->get_player_name());
		player->set_totalchips((*iter).get_seated_player()->get_chips());
		player->set_status((*iter).get_seated_player()->get_status());
		player->set_seatnumber(iter->get_seat_number());
		if (player->status() != Player::PLAYER_WAIT_NEXT_GAME && player->status() != Player::PLAYER_UNREADY
			&& player->status() != Player::PLAYER_STAND_UP)
		{
			player->set_score(iter->get_score());
			copy_group_to_msg(iter->get_card_group(), player->mutable_cardgroup());
		}
		else
		{
			player->set_score(0);
		}
		player->set_handcardcount(iter->get_hand_cards_total().size());
	}
}

//return -1不能站起
int GameDesk::player_stand_up(const PLAYER_PTR& player, int stand_up_reason)
{
	TRACE_FUNCATION();
	assert(NULL != player && player->get_seat_number() >= 0 && player->get_seat_number() <= (m_max_seat_num - 1));
	assert(player == m_seats[player->get_seat_number()].get_seated_player());
	
	bool is_ingame = m_seats[player->get_seat_number()].is_in_game();

	if (!is_game_over() && is_ingame)
	{
		if (is_declareing())
		{
			//赢家必须等到所有玩家都declare才能站起
			if (player->get_seat_number() == m_apply_declare_seatnum)
			{
				DEBUG_LOG << "user(" << player->get_user_id() << ") is winner must wait all player declare then can leave";
				return -1;
			}
			if (player->get_seat()->is_in_game())
			{
				//先declare 再站起
				default_decalre(player);
			}
		}
		
		else
		{
			DEBUG_LOG <<  "user(" << player->get_user_id() << ") must drop first then can leave";
			return -1;
		}
	}

	if (player->get_seat_number() == m_last_winner_seatnum)
	{
		m_last_winner_seatnum = -1;
	}

	bool is_playing = player->is_player_playing();

	broadcast_player_standup(player, stand_up_reason);
	player->get_seat()->player_stand_up();

	if (is_distribut() && is_playing)
	{
		if (is_all_player_over_account())
		{
			wait_for_round(GAME_DISTRIBUTE, 0);
			set_player_matchover_status();
			DEBUG_LOG << "user(" << (player->get_user_id()) << ") stand_up room_status:" << get_status_name(get_game_status());
			return 0;
		}
	}

	if (!is_game_can_start() && is_game_wait_playing())
	{
		notify_player_wait_nextgame(0);
		wait_for_round(GAME_UNREADY, 0);
		notify_room_status(GG_FlushRoomStausRequest::ROOM_IDLE);
		cancle_notify_timer();
	}

	return 0;
}

void GameDesk::cancle_notify_timer()
{
	m_notify_status_timer.cancel();
}


int GameDesk::get_dealer_index(void)
{
	return m_dealer_index;
}


int GameDesk::get_seated_count(bool include_npc)
{
	int count = 0;
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (iter->is_seated())
		{
			++count;
		}
	}
	return count;
}


int GameDesk::get_ingame_count(void)
{
	int count = 0;
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (iter->is_in_game())
		{
			++count;
		}
	}
	return count;
}

bool GameDesk::is_game_over(void)
{
	DEBUG_LOG << "game_is_unready:" << (m_game_status & GAME_UNREADY)
		<< " game_is_waiting:" << (is_waiting_for_round())
		<< " game_is_playing:" << (m_game_status & GAME_WAIT_PLAYING)
		<< " game_is_distribute:" << (m_game_status & GAME_DISTRIBUTE);
	return (m_game_status & GAME_UNREADY) || (m_game_status & GAME_WAIT_PLAYING) || 
		(m_game_status & GAME_DISTRIBUTE);
}

bool GameDesk::is_game_can_start(void)
{
    return (get_seated_count() > 1); 
}


bool GameDesk::is_waiting_for_round(void)
{
    return m_game_status & GAME_WAIT_MASK;
}


void GameDesk::start_new_game(void)
{
	TRACE_FUNCATION();

    step_desk_ready();
    
    on_newgame_start();
	
	step_dealer_seat();

	assert(m_dealer_index != -1);

	// 发牌
    step_deal_cards();

    wait_for_round(GAME_DEALER_CARD, m_room->get_game_wait_interval().m_time_deal_card);

	m_time_game_begin = time(NULL);

}

unsigned int GameDesk::get_game_status(void)
{
	return m_game_status;
}

int GameDesk::next_seat_number(int seat_number)
{
	return (seat_number + 1) % m_max_seat_num;
}

int GameDesk::next_seated_number(int seat_number)
{
	TRACE_FUNCATION();
	int start_seat = seat_number;
	int next_seat = next_seat_number(start_seat);
	while (next_seat != start_seat)
	{
		if (m_seats[next_seat].is_seated())
		{
			return next_seat;
		}
		else
		{
			next_seat = next_seat_number(next_seat);
		}
	}
	return next_seat;
}


void GameDesk::step_deal_cards(void)
{
	TRACE_FUNCATION();
	m_draw_sec = 20 * 60;
	int left_count = 0;
	int count = CardDealer::CARD_COUNT;
	m_bottom_cards.clear();
	m_room->deal_card(m_bottom_cards, count);

	int is_from_file = Game_Process::instance()->get_config().GetItemInt("gamesvr", "card_from_file", "0");

	if (is_from_file)
	{
		vector<PokerCard> all_card;
		all_card.swap(m_bottom_cards);
		get_card_from_file(m_bottom_cards, all_card, m_draw_sec, left_count, m_room->get_room_info().m_small_blind);
	}

	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (!iter->is_seated())
		{
			continue;
		}
		else
		{
			std::vector<PokerCard> handcards;
			for (int i = 0; i < CardDealer::HAND_CARD_COUNT; ++i)
			{
				handcards.push_back(m_bottom_cards.front());
				m_bottom_cards.erase(m_bottom_cards.begin());
			}
			iter->deal_cards(handcards);
		}
	}

	//定癞子牌和翻牌
	PokerCard first_card = m_bottom_cards.front();
	m_bottom_cards.erase(m_bottom_cards.begin());
	m_pokercard_pool.push_back(first_card);
	std::vector<PokerCard>::iterator it = m_bottom_cards.begin();
	for (; it != m_bottom_cards.end(); ++it)
	{
		if (it->m_point >= PokerCard::JOKER1)
		{
			continue;
		}
		else
		{
			break;
		}
	}
	//翻癞子牌
	
	m_joker_card = *it;
	m_bottom_cards.erase(it);
	if (left_count > 0)
	{
		m_bottom_cards.erase(m_bottom_cards.begin() + left_count, m_bottom_cards.end());
	}
	
	LOG_DEBUG(" bottom_count:" + toString(m_bottom_cards.size()));
	m_room->broadcast_deal_cards();
	
	m_game_log_record.deal_cards(m_seats);
	send_flush_user_ingame();
}

void GameDesk::get_player_deal_order(google::protobuf::RepeatedField<google::protobuf::int32>* order)
{
	TRACE_FUNCATION();

	int seat = m_dealer_index;
	do
	{
		if (m_seats[seat].is_seated())
		{
			google::protobuf::int32* pSeat = order->Add();
			*pSeat = seat;
		}

		seat = next_seat_number(seat);
	} while (seat != m_dealer_index);
}

void GameDesk::set_game_status(unsigned int status)
{
	LOG_INFO("room(" + toString(m_room->get_room_info().m_area_id) + ") status(" + get_status_name(m_game_status) + " -> " + get_status_name(status) + ")");
	m_game_status = status;
}

void GameDesk::broadcast_player_standup(const PLAYER_PTR& player, int stand_up_reason)
{
	TRACE_FUNCATION();
	m_room->broadcast_player_standup(player, stand_up_reason);
}

void GameDesk::step_desk_ready(void)
{
	TRACE_FUNCATION();
	reactor()->cancel_timer(this);
	m_wait_round_timerid = -1; //当前等待定时器ID
	m_pokercard_pool.clear();
	m_is_first_pool_card = true;

	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		iter->ready_for_game();
	}
}

void GameDesk::clear_desk(void)
{
	m_bottom_cards.clear();
	m_joker_card = PokerCard();
	m_pokercard_pool.clear();
	m_chips_pool = 0;
	m_apply_declare_seatnum = -1;
	m_active_seatnum = -1;
	m_gameover_type = -1;
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		iter->reset_seat();
	}
}

int GameDesk::check_player(const PLAYER_PTR& player)
{
	if (!is_seated_player(player))
	{
		LOG_WARN("room(" + toString(m_room->get_room_info().m_area_id) + ") user(" + toString(player->get_user_id()) + ") is not seated player");
		return -1;
	}
	return 0;
}

bool GameDesk::is_seated_player(const PLAYER_PTR& player)
{
	TRACE_FUNCATION();
	if (!player->is_seated_player())
	{
		LOG_WARN("room(" + toString(m_room->get_room_info().m_area_id) + ") user(" + toString(player->get_user_id()) + ") is not seated player");
		return false;
	}

	if (player != m_seats[player->get_seat_number()].get_seated_player())
	{
		LOG_WARN("room(" + toString(m_room->get_room_info().m_area_id) + ") user(" + toString(player->get_user_id()) + ") is not seated player");
		return false;
	}
	return true;
}

void GameDesk::check_player_status()
{
	for (vector<GameSeat>::iterator it = m_seats.begin(); it != m_seats.end(); ++it)
	{
		if (!it->is_seated())
		{
			continue;
		}
		const PLAYER_PTR&  player = it->get_seated_player();
		if (player->is_player_need_standup())
		{
			LOG_DEBUG("player(" + toString(player->get_user_id()) + ") stand up for check_player_status failed");
			player_stand_up(player, PlayerStandUpRequest::OTHER_REASON);
			m_room->destroy_player(player);
		}
	}
}

void GameDesk::notify_player_wait_nextgame(int flag)
{
	GG_NotifyWaitGameRequest request;
	
	request.set_flag(flag);
	int seat_count = get_seated_count();
	if (seat_count == 2)
	{
		m_wait_game_sec = 10;
	}
	else if (seat_count == 3)
	{
		m_wait_game_sec = 8;
	}
	else
	{
		m_wait_game_sec = 5;
	}
	request.set_seconds(m_wait_game_sec);
	m_room->broadcast_message(GG_CMD_NOTIFY_WAIT_GAME, request);
}

void GameDesk::step_unready()
{
	TRACE_FUNCATION();
	check_player_chips();
	if (is_game_can_start())
	{
		notify_player_wait_nextgame(1);
		m_wait_begin = time(NULL);
		wait_for_round(GAME_WAIT_PLAYING, ACE_Time_Value(m_wait_game_sec));
		cancle_notify_timer();
		m_notify_room_timerid = reactor()->schedule_timer(this, &m_notify_room_timerid,
			ACE_Time_Value(m_wait_game_sec) - ACE_Time_Value(2, 0));
		if (m_notify_room_timerid == -1)
		{
			LOG_ERROR("register timer failed");
		}
	}
}

int GameDesk::notify_room_will_over(int cur_score)
{
	if (cur_score <= ENTER_SCORE1 && cur_score >= 0 && m_notify_room_type == GG_FlushRoomStausRequest::ROOM_PLAYING)
	{
		notify_room_status(GG_FlushRoomStausRequest::ROOM_END1);
	}
}

void GameDesk::step_playing_begin()
{
	//能开始牌局
	if (!is_game_can_start())
	{
		notify_room_status(GG_FlushRoomStausRequest::ROOM_IDLE);
		wait_for_round(GAME_UNREADY, ACE_Time_Value::zero);
	}
	else
	{
		notify_room_status(GG_FlushRoomStausRequest::ROOM_PLAYING);
		start_new_game();
	}
}

void GameDesk::step_new_round(void)
{
	TRACE_FUNCATION();
	unsigned int wait_round = get_game_status() & GAME_ROUND_MASK;
	set_game_status(m_game_status & ~GAME_WAIT_MASK);
	LOG_DEBUG("game[" + toString(m_game_id) + "]'s wait round is " + get_status_name(wait_round));
	LOG_DEBUG("room(" + toString(m_room->get_room_info().m_area_id) + ") wait_round:" + toString(wait_round));
	switch (wait_round)
	{
    case GAME_UNREADY:
        {
			step_unready();
			break;
		}

	case GAME_WAIT_PLAYING:
		{
			step_playing_begin();
			break;
		}
	case GAME_DEALER_CARD:
		{
			active_player();
			break;
		}
 
	case GAME_ACTIVE1:
		{
			step_active1_timeout();
			break;
		}
	
	case GAME_ACTIVE2:
		{
			step_rand2_timeout();
			break;
		}
		
	case GAME_DECLARE:
		{
			LOG_DEBUG("  step_declare_timeout");
			step_declare_timeout();
			break;
		}

	case GAME_SHUFFLE_CARD:
		{
			LOG_DEBUG("GAME_SHUFFLE_CARD timeout");
			active_player();
			break;
		}

	case GAME_DISTRIBUTE:
		{
			ready_for_next_game();
			break;
		}
	default:
		break;
	}
}

void GameDesk::step_declare_timeout()
{
	TRACE_FUNCATION();
	for (std::vector<GameSeat>::iterator it = m_seats.begin(); it != m_seats.end(); ++it)
	{
		if (it->is_seated() && it->is_in_game() && !it->is_declared())
		{
			default_decalre(it->get_seated_player());
		}
		else
		{
			LOG_DEBUG("step_declare_timeout seat:" + toString(it->is_seated()) + "," + toString(it->is_in_game()) + ","
			+ toString(!it->is_declared()));
		}
	}
}

void GameDesk::step_rand2_timeout()
{
	//默认出一张最大的牌
	TRACE_FUNCATION();
	const PLAYER_PTR&  player = m_seats[m_active_seatnum].get_seated_player();
	player->set_trustee_on();
	m_game_log_record.wait2_palyer_timeout(player->get_user_id());
	post_default_card();
	//active_player(m_room->get_game_wait_interval().m_time_post_card);
}

int GameDesk::next_ingame_seat_number(int number)
{
	TRACE_FUNCATION();
	int start_seat = number;
	int next_seat = next_seat_number(start_seat);
	while (next_seat != start_seat)
	{
		if (m_seats[next_seat].is_in_game())
		{
			return next_seat;
		}
		else
		{
			next_seat = next_seat_number(next_seat);
		}
	}
	return next_seat;
}

void GameDesk::active_player(int extra_time)
{
	TRACE_FUNCATION();
	if (m_active_seatnum == -1)
	{
		m_active_seatnum = m_dealer_index;
	}
	else
	{
		m_active_seatnum = next_ingame_seat_number(m_active_seatnum);
	}

	time_t tnow = time(NULL);
	if ( m_time_game_begin + m_draw_sec < tnow)
	{
		m_gameover_type = BORNING_OVER;
		notify_room_status(GG_FlushRoomStausRequest::ROOM_IDLE);
		deduct_draw_tips();
		step_gameover_distrbute();
		return;
	}
	GG_ActivePlayerRequest request;
	request.set_userid(m_seats[m_active_seatnum].get_seated_player()->get_user_id());
	if (CombinationInstance::get_const_instance().is_joker(m_pokercard_pool.back(), m_joker_card.m_point) && !m_is_first_pool_card)
	{
		request.set_ispoolcardcandig(0);
	}
	else
	{
		request.set_ispoolcardcandig(1);
	}
	
	m_room->broadcast_message(GG_CMD_ACTIVE_PLAYER, request);
	const PLAYER_PTR&  player = m_seats[m_active_seatnum].get_seated_player();
	if (m_seats[m_active_seatnum].get_seated_player()->is_player_trusteed_on())
	{ 
		wait_for_round(GAME_ACTIVE1, extra_time + 1000);
	}
	else
	{
		wait_for_round(GAME_ACTIVE1, m_room->get_game_wait_interval().m_time_active1 + extra_time + 1500);
	}
	m_wait_begin = time(NULL);
	m_game_log_record.active1_player(m_active_seatnum, player->get_user_id());
}

void GameDesk::ready_for_next_game()
{
	wait_for_round(GAME_UNREADY, 0);
	check_player_status();
	set_player_matchover_status();
}

std::string GameDesk::get_status_name(unsigned int status) const
{
	std::string status_name;
	if (status & GAME_WAIT_MASK)
	{
		status_name = "WAIT_FOR* ";
		status &= ~GAME_WAIT_MASK;
	}

	switch (status & GAME_ROUND_MASK)
	{
    case GAME_UNREADY:
        {
            status_name += "GAME_UNREADY";
            break;
        }
	case GAME_WAIT_PLAYING:
		{
			status_name += "GAME_WAIT_PLAYING";
			break;
		}
	case GAME_DEALER_CARD:
		{
			status_name += "GAME_DEALER_CARD";
			break;
		}

    case GAME_ACTIVE1:
        {
            status_name += "GAME_ACTIVE1";
            break;
        }
	case GAME_ACTIVE2:
		{
			status_name += "GAME_ACTIVE2";
			break;
		}
	case GAME_DECLARE:
	{
		status_name += "GAME_DECLARE";
		break;
	}
	case GAME_DISTRIBUTE:
		{
			status_name += "GAME_DISTRIBUTE";
			break;
		}
	case GAME_SHUFFLE_CARD:
	{
		status_name += "GAME_SHUFFLE_CARD";
		break;
	}
	default:
		{
			status_name += "UNKNOWN:" + toString(status & GAME_ROUND_MASK);
			break;
		}
	}

	return status_name;
}

GAMEROOM_PTR& GameDesk::get_room(void) const
{
	return m_room;
}

void GameDesk::wait_for_round(unsigned int round, int wait_ms)
{
	TRACE_FUNCATION();
	set_game_status((GAME_WAIT_MASK | round) | (m_game_status & ~GAME_ROUND_MASK));
	m_wait_round_timer.expires_from_now(boost::posix_time::milliseconds(wait_ms));
	m_wait_round_timer.async_wait(std::bind)
}

int GameDesk::get_idle_seat_number(void)
{
	TRACE_FUNCATION();
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (!iter->is_seated())
		{
			return iter->get_seat_number();
		}
	}
	return -1;
}

void GameDesk::submit_game_result(void)
{
	TRACE_FUNCATION();

	Json::Value jv;
	jv[RPC_METHOD] = "notify_player_gameover";
	jv[RPC_PARAMETER]["room_id"] = m_room->get_room_info().m_area_id;
	jv[RPC_PARAMETER]["room_type"] = m_room->get_room_info().m_room_type;
	jv[RPC_PARAMETER]["desk_type"] = m_room->get_room_info().m_desk_type;
	jv[RPC_PARAMETER]["small_blind"] = toString(m_room->get_room_info().m_small_blind);


	GG_SubmitGameResultRequest request;
	request.set_gameid(m_game_log_record.get_game_id());

	TY_INT64 small_blind = get_room()->get_room_info().m_small_blind;

	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (!iter->is_player_joined())
		{
			continue;
		}

		PlayerGameResult* game_result = request.mutable_gameresult()->Add();
		game_result->set_userid(iter->get_ingame_player_id()); 
		game_result->set_chipsincrease(iter->get_match_chips_account());
		
	
		if (iter->is_winner())
		{
			game_result->set_expincrease(m_room->get_room_info().m_base_exp_win);
			
		}
		else
		{
			game_result->set_expincrease(m_room->get_room_info().m_base_exp_lose);
		}

		game_result->set_statusmask(iter->seat_status() & (GameSeat::win));
		
		jv[RPC_PARAMETER]["user_id"].append(iter->get_ingame_player_id());
	} 
	m_game_log_record.game_result(request);
    m_game_log_record.submit_game_log();
	MQService::instance()->send_mqrequest(GG_CMD_SUBMIT_GAME_RESULT, MQ_TRANSMIT_DIRECT, SITE_NAME_DCENTERSVR + "." + Game_Process::instance()->getGameName(), request);
   
	GG_RPCRequest notify_req;
	notify_req.set_jsonstring(json_to_string(jv));
	MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, notify_req);
}

void GameDesk::on_newgame_start(void)
{
	m_game_log_record.set_game_id(make_game_id());
	m_game_log_record.set_seat_player(m_seats);
    send_user_box_time_recover();
}

std::string GameDesk::make_game_id(void)
{
	std::string game_id = toString(m_room->get_room_info().m_area_id) + "_";
	time_t t = time(NULL);
	struct tm tmNow = Game_Process::instance()->local_time(t);
	char buf[PATH_MAX] = {0};
	ACE_OS::sprintf(buf, "%04d%02d%02d%02d%02d%02d", tmNow.tm_year + 1900, tmNow.tm_mon + 1, tmNow.tm_mday, tmNow.tm_hour, tmNow.tm_min, tmNow.tm_sec);
	game_id += buf;
	m_game_id = game_id;
	return game_id;
}

std::shared_ptr<GG_GameLogRecord>& GameDesk::get_game_log_record(void) const
{
	return &m_game_log_record;
}

int GameDesk::get_start_deal_card_count(void)
{
	return CardDealer::HAND_CARD_COUNT;
}

void GameDesk::send_flush_user_ingame(void)
{
	GG_FlushUserInGameRequest request;
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		const PLAYER_PTR& player = iter->get_seated_player();
		if (!iter->is_seated() || NULL == player)
		{
			continue;
		}

		GG_UserGameStatus* userStatus = request.mutable_gameplayer()->Add();
		userStatus->set_userid(player->get_user_id());
		userStatus->set_gamestatus(player->get_status() == Player::PLAYER_STAND_UP ? GG_FlushUserInGameRequest::USER_IN_WATCH : GG_FlushUserInGameRequest::USER_IN_GAME);
		userStatus->set_roomid(player->get_room()->get_room_info().m_area_id);

	}
	MQService::instance()->send_mqrequest(GG_CMD_FLUSH_USER_IN_GAME, MQ_TRANSMIT_DIRECT, SITE_NAME_NOTIFYSVR, request);
}

void GameDesk::cancel_wait_round(void)
{
    if (-1 != m_wait_round_timerid)
    {
        reactor()->cancel_timer(m_wait_round_timerid);
        m_wait_round_timerid = -1;
    }
}


void GameDesk::check_player_chips(void)
{
    for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
    {
        const PLAYER_PTR& player = iter->get_seated_player();
        if (player && !player->check_chips_carry())
        {
			LOG_DEBUG("player(" + toString(player->get_user_id()) + ") stand up for check_chips_carry failed");
            player_stand_up(player, PlayerStandUpRequest::NO_MORE_CHIPS);
			m_room->destroy_player(player);
        }
    }
}


int GameDesk::player_flush_room(const PLAYER_PTR& player, const MESSAGE_PTR &msg)
{
    TRACE_FUNCATION();
	GG_FlushRoomRequest request;
	request.set_roomtype(m_room->get_room_info().m_room_type);
	request.set_smallblind(m_room->get_room_info().m_small_blind);
	request.set_mincarry(m_room->get_room_info().m_min_carry);
	request.set_gamestatus(get_game_status_for_app());
	request.set_maxplaying(m_room->get_room_info().m_max_playing);
	
	request.set_desktype(m_room->get_room_info().m_desk_type);
	CopyCardToMsg(m_pokercard_pool, request.mutable_cardpool());
	request.set_bottomcardcount(is_game_over() ? CardDealer::CARD_COUNT : m_bottom_cards.size());
	m_joker_card.to_msg_card(request.mutable_jokercard());
	request.set_chipspool(m_chips_pool);
	request.set_dealerindex(m_dealer_index);
	request.set_applydeclareseatnum(m_apply_declare_seatnum);
	request.set_active1sec(m_room->get_game_wait_interval().m_time_active1/1000);
	request.set_active2sec(m_room->get_game_wait_interval().m_time_active2/1000);
	request.set_distributsec(m_room->get_game_wait_interval().m_time_distribute/1000);
	request.set_declaresec(m_room->get_game_wait_interval().m_time_declare/1000);
	request.set_waitgamesec(m_room->get_game_wait_interval().m_time_wait_next_game/1000);
	request.set_waitgamesec(m_room->get_game_wait_interval().m_time_declare/1000);
	
	request.set_dropscore1(20);
	request.set_dropscore2(40);

	if (m_pokercard_pool.empty() || (CombinationInstance::get_const_instance().is_joker(m_pokercard_pool.back(), m_joker_card.m_point) && !m_is_first_pool_card))
	{
		request.set_ispoolcardcandig(0);
	}
	else
	{
		request.set_ispoolcardcandig(1);
	}
	
	if (player->is_seated_player() && player->get_seat()->is_in_game())
	{
		CopyCardToMsg(player->get_seat()->get_hand_cards_total(), request.mutable_handcard());
		
	}
	if (is_wait_active())
	{
		request.set_currentactiveseat(m_active_seatnum);
	}
	else
	{
		request.set_currentactiveseat(-1);
	}

	int tLeft = 0;

	if (is_wait_active1())
	{
		tLeft = m_room->get_game_wait_interval().m_time_active1.sec() - (time(NULL) - m_wait_begin);
		if (tLeft > m_room->get_game_wait_interval().m_time_active1.sec())
		{
			tLeft = m_room->get_game_wait_interval().m_time_active1.sec();
		}
	}
	else if (is_wait_active2())
	{
		tLeft = m_room->get_game_wait_interval().m_time_active2.sec() - (time(NULL) - m_wait_begin);
		if (tLeft > m_room->get_game_wait_interval().m_time_active2.sec())
		{
			tLeft = m_room->get_game_wait_interval().m_time_active2.sec();
		}
	}

	else if (is_distribut())
	{
		tLeft = (m_room->get_game_wait_interval().m_time_distribute 
			+ m_room->get_game_wait_interval().m_time_account_animat).sec() - (time(NULL) - m_wait_begin);
		LOG_DEBUG("cur_status:" + get_status_name(m_game_status) + " time_left:" + toString(tLeft));
	}

	else if (is_declareing())
	{
		tLeft = m_room->get_game_wait_interval().m_time_declare.sec() - (time(NULL) - m_wait_begin);
	}

	else if (is_wait_game())
	{
		tLeft = m_wait_game_sec - (time(NULL) - m_wait_begin);
		if (tLeft > m_wait_game_sec)
		{
			tLeft = m_wait_game_sec;
		}
		LOG_DEBUG("cur_status:" + get_status_name(m_game_status) + " time_left:" + toString(tLeft));
	}
	if (tLeft <= 0)
	{
		tLeft = 1;
	}
	LOG_DEBUG("cur_status:" + get_status_name(m_game_status) + " time_left:" + toString(tLeft));
	request.set_timeleft(tLeft);
	get_seat_player(request.mutable_player());
	get_joingame_player(request.mutable_ingameplayer());
	player->send_request(GG_CMD_FLUSH_ROOM, request);
    return 0;
}

TY_UINT8 GameDesk::get_game_status_for_app(void)
{
	TY_UINT32 gamestatus = get_game_status();
	TY_UINT32 game_round = gamestatus & GameDesk::GAME_ROUND_MASK;
	TY_UINT8 ret_status = GG_FlushRoomRequest::GAME_UNREADY;

    if (gamestatus & GameDesk::GAME_WAIT_MASK)
    {

        switch (game_round)
        {
        case GameDesk::GAME_UNREADY:
            {
                ret_status = GG_FlushRoomRequest::GAME_UNREADY;
                break;
            }
		case GameDesk::GAME_WAIT_PLAYING:
			{
				ret_status = GG_FlushRoomRequest::GAME_WAIT_PLAYING;
				break;
			}
		case GameDesk::GAME_DEALER_CARD:
			{
				ret_status = GG_FlushRoomRequest::GAME_DEALER_CARD;
				break;
			}
		case GameDesk::GAME_ACTIVE1:
			{
				ret_status = GG_FlushRoomRequest::GAME_ACTIVE1;
				break;
			}
		case GameDesk::GAME_ACTIVE2:
			{
				ret_status = GG_FlushRoomRequest::GAME_ACTIVE2;
				break;
			}
		case GameDesk::GAME_DECLARE:
			{
				ret_status = GG_FlushRoomRequest::GAME_DECLARE;
				break;
			}
		case GameDesk::GAME_DISTRIBUTE:
			{
				ret_status = GG_FlushRoomRequest::GAME_DISTRIBUTE;
				break;
			}

		case GameDesk::GAME_SHUFFLE_CARD:
			{
				ret_status = GG_FlushRoomRequest::GAME_SHUFFLE_CARD;
				break;
			}

        default:
            {
                WARN_LOG << "room(" << (m_room->get_room_info().m_room_id) << ") unknown round(" << game_round << ")";
                ret_status = GG_FlushRoomRequest::GAME_UNREADY;
                break;
            }
        }
    }
    else
    {
        // 应该只 会看到 GameDesk::GAME_UNREADY 才对， 其它的都是瞬时状态
       // assert(GameDesk::GAME_UNREADY == game_round);
		DEBUG_LOG << "get_status_for_app:" << gamestatus;;
        ret_status = GG_FlushRoomRequest::GAME_UNREADY;
    }
    return ret_status;
}


void GameDesk::send_user_box_time_recover(void)
{
    for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
    {
        const PLAYER_PTR& player = iter->get_seated_player();
        if (!iter->is_seated() || NULL == player)
        {
            continue;
        }

        player->send_user_box_time_recover();
    }
}

void GameDesk::send_user_box_time_pause(void)
{
    for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
    {
         if (iter->is_in_game())
        {
            const PLAYER_PTR& player = iter->get_seated_player();
            player->send_user_box_time_pause();
        }
    }
}

TY_INT64 GameDesk::get_sitdown_buyin_count(void)
{
	return m_room->get_room_info().m_min_carry;
}

bool GameDesk::is_game_unready()
{
	return m_game_status & GAME_UNREADY;
}



vector<PokerCard>::iterator find_card(vector<PokerCard> &cards, const PokerCard& pc)
{
	for (vector<PokerCard>::iterator it = cards.begin(); it != cards.end(); ++it)
	{
		if (it->m_color == pc.m_color && it->m_point == pc.m_point)
		{
			return it;
		}
	}
	return cards.end();
}

void get_card_from_file(vector<PokerCard> &vec_card, vector<PokerCard> &all_card, 
	int& draw_sec, int& left_bottom_count, TY_INT64 smallblind)
{
	TRACE_FUNCATION();
	vec_card.clear();
	std::ifstream ifile("hand_card_rummy.txt");
	if (ifile.fail())
	{
		vec_card = all_card;
		ifile.close();
		LOG_WARN("open file hand_card_rummy.txt failed");
		return;
	}
	string fileData((istreambuf_iterator<char>(ifile)),
		istreambuf_iterator<char>());
	Json::Value jv = parse_json_string(fileData);
	if (jv.isNull() || jv["available"].isNull() || jv["cards"].isNull() || !jv["cards"].isArray())
	{
		vec_card = all_card;
		LOG_WARN("hand_card_rummy.txt no available")
		return;
	}

	bool bAvailable = jv["available"].asBool();
	if (!bAvailable)
	{
		vec_card = all_card;
		LOG_WARN("hand_card_rummy.txt no available")
		return;
	}

	std::set<TY_INT64> blinds;
	if (!jv["small_blind"].isNull() && jv["small_blind"].isArray() && !jv["small_blind"].empty())
	{
		for (Json::UInt i = 0; i < jv["small_blind"].size(); ++i)
		{
			blinds.insert(BaseTool::atoi64(jv["small_blind"][i].asString()));
		}
		if (blinds.find(smallblind) == blinds.end())
		{
			LOG_WARN("hand_card_rummy.txt no blind")
			vec_card = all_card;
			return;
		}
	}

	if (!jv["draw_sec"].isNull() && jv["draw_sec"].isInt())
	{
		draw_sec = jv["draw_sec"].asInt();
	}

	if (!jv["left_count"].isNull() && jv["left_count"].isInt())
	{
		left_bottom_count = jv["left_count"].asInt();
	}

	Json::Value &jvArray = jv["cards"];
	vector<PokerCard> vecHandCard[5];
	vector<PokerCard> bottomCard;
	for (size_t i = 0; i < 5 && i < jvArray.size(); ++i)
	{
		Json::Value &jvOneCards = jvArray[i];
		for (size_t j = 0; j < jvOneCards.size() && j < CardDealer::HAND_CARD_COUNT; ++j)
		{
			PokerCard pc;
			pc.parse_string(jvOneCards[j].asString());
			std::vector<PokerCard>::iterator it = find_card(all_card, pc);
			if (it == all_card.end())
			{
				LOG_ERROR("cards error");
				vec_card = all_card;
				return;
			}
			vecHandCard[i].push_back(*it);
			all_card.erase(it);
		}
	}

	Json::Value &jvBottom = jv["bottom_cards"];
	for (size_t i = 0; i < jvBottom.size(); ++i)
	{
		PokerCard pc;
		pc.parse_string(jvBottom[i].asString());
		std::vector<PokerCard>::iterator it = find_card(all_card, pc);
		if (it == all_card.end())
		{
			LOG_ERROR("cards error");
			vec_card = all_card;
			return;
		}
		bottomCard.push_back(*it);
		all_card.erase(it);
	}

	for (size_t i = 0; i < 5 && i < jvArray.size(); ++i)
	{
		for (size_t j = 0; j < CardDealer::HAND_CARD_COUNT; ++j)
		{
			if (j < vecHandCard[i].size())
			{
				vec_card.push_back(vecHandCard[i][j]);
			}
			else
			{
				vec_card.push_back(all_card.front());
				all_card.erase(all_card.begin());
			}
		}
	}
	std::copy(bottomCard.begin(), bottomCard.end(), std::back_inserter(vec_card));
	std::copy(all_card.begin(), all_card.end(), std::back_inserter(vec_card));
	LOG_DEBUG(CombinationInstance::get_const_instance().cards_to_string(vec_card));
}

void GameDesk::caculate_over_account(GG_GameOverRequest& request)
{
	m_last_winner_seatnum = m_apply_declare_seatnum;
	request.set_gameovertype(m_gameover_type);
	if (m_apply_declare_seatnum == -1)
	{
		request.set_winnerid(-1);
	}
	else
	{
		TY_INT64 tips_persent = m_room->get_game_desk_tips(m_chips_pool);
		
		TY_INT64 real_chips = m_chips_pool - tips_persent;
		real_chips = m_seats[m_apply_declare_seatnum].match_over_account(real_chips);
		request.set_winaccount(real_chips);
		request.set_winnerid(m_seats[m_apply_declare_seatnum].get_ingame_player_id());
		copy_group_to_msg(m_seats[m_apply_declare_seatnum].get_card_group(), request.mutable_winnercardgroup());
		request.set_winnerscore(0);
		request.set_winnerscore(m_seats[m_apply_declare_seatnum].get_card_score());
		DEBUG_LOG << "user(" << request.winnerid() << ") score(" << request.winnerscore() << 
			") card_score(" << request.winnercardscore() << ") chips(" << request.winaccount() << ")";
		m_game_log_record.deduct_desk_tips(request.winnerid(),tips_persent);
	}
	
	for (std::vector<GameSeat>::iterator it = m_seats.begin(); it != m_seats.end(); ++it)
	{
		if (!it->is_player_joined())
		{
			continue;
		}
		if (it->get_seat_number() == m_apply_declare_seatnum)
		{
			PlayerChips* pc = request.add_playerchips();
			pc->set_userid(it->get_ingame_player_id());
			if (it->is_seated() && it->get_ingame_player_id() == it->get_seated_player()->get_user_id())
			{
				pc->set_currentchips(it->get_seated_player()->get_chips());
			}
			else
			{
				pc->set_currentchips(0);
			}
			continue;
		}
		GG_LoserInfo *info = request.add_loserinfo();
		info->set_userid(it->get_ingame_player_id());
		info->set_isdrop(it->is_drop() ? 1 : 0);
		info->set_loseaccount(it->get_match_chips_account());
		if (m_apply_declare_seatnum == -1)
		{
			it->borning_over();
		}
		info->set_score(it->get_score());
		info->set_cardscore(it->get_card_score());
		DEBUG_LOG << "user(" << it->get_ingame_player_id() << ") score(" << info->score() << ") card_score(" << info->cardscore() << ")";
		copy_group_to_msg(it->get_card_group(), info->mutable_cardgroup());

		PlayerChips* pc = request.add_playerchips();
		pc->set_userid(it->get_ingame_player_id());
		if (it->is_seated() && it->get_ingame_player_id() == it->get_seated_player()->get_user_id())
		{
			pc->set_currentchips(it->get_seated_player()->get_chips());
		}
		else
		{
			pc->set_currentchips(0);
		}
		DEBUG_LOG << "user(" << info->userid()<< ") score(" << info->score() << 
			") card_score(" << info->cardscore() << ") chips(" << info->loseaccount() << ")";
	}
}


void GameDesk::set_ingame_player_over_status()
{
	for (vector<GameSeat>::iterator it = m_seats.begin(); it != m_seats.end(); ++it)
	{
		if (!it->is_seated())
		{
			continue;
		}
		int player_status = it->get_seated_player()->get_status() & ~Player::PLAYER_MASK;
		if (it->is_player_joined() && player_status != Player::PLAYER_UNREADY && player_status != Player::PLAYER_WAIT_NEXT_GAME)
		{
			it->get_seated_player()->set_status(Player::PLAYER_PLAYING |
				(it->get_seated_player()->get_status()  & ~Player::PLAYER_MASK));
		}

	}
}

void GameDesk::set_player_matchover_status()
{
	for (vector<GameSeat>::iterator it = m_seats.begin(); it != m_seats.end(); ++it)
	{
		if (!it->is_seated())
		{
			continue;
		}
		if (it->is_player_joined())
		{
			it->get_seated_player()->set_status(Player::PLAYER_UNREADY | 
				(it->get_seated_player()->get_status()  & ~Player::PLAYER_MASK));
		} 
		
	}
}

void GameDesk::step_dealer_seat(void)
{
	if (m_last_winner_seatnum != -1)
	{
		if (m_seats[m_last_winner_seatnum].is_seated())
		{
			m_dealer_index = m_last_winner_seatnum;
			return;
		}
	}

	std::vector<TY_INT8> seated_seats;
	for (std::vector<GameSeat>::iterator it = m_seats.begin(); it != m_seats.end(); ++it)
	{
		if (it->is_seated())
		{
			seated_seats.push_back(it->get_seat_number());
		}
	}
	assert(!seated_seats.empty());
	m_dealer_index = seated_seats[rand() % seated_seats.size()];
	DEBUG_LOG << "dealer index:" << m_dealer_index;
}

void GameDesk::step_active1_timeout()
{
	TRACE_FUNCATION();
	const PLAYER_PTR&  player = m_seats[m_active_seatnum].get_seated_player();
	assert(player != NULL);
	m_game_log_record.wait1_palyer_timeout(player->get_user_id());
	player->set_trustee_on();
	active_player(ACE_Time_Value::zero);


	//帮玩家摸一张牌
	//player_dig_card(player, DIG_BOTTOM);
}

void GameDesk::step_shuffle_card()
{
	TRACE_FUNCATION();
	std::vector<PokerCard> cards;
	std::copy(m_pokercard_pool.begin(), m_pokercard_pool.end() - 2, std::back_inserter(cards));
	m_pokercard_pool.erase(m_pokercard_pool.begin(), m_pokercard_pool.end() - 2);
	while (!cards.empty())
	{
		int index = rand() % cards.size();
		m_bottom_cards.push_back(cards[index]);
		if (cards.size() > 1)
		{
			cards[index] = cards.back();
		}
		cards.pop_back();
	}

	GG_ShuffleCardRequest request;
	request.set_bottomcardcount(m_bottom_cards.size());
	request.set_poolcardcount(m_pokercard_pool.size());
	m_room->broadcast_message(GG_CMD_SHUFFLE_CARD, request);
}

void GameDesk::player_dig_card(const PLAYER_PTR&  player, const MESSAGE_PTR &msg)
{
	TRACE_FUNCATION();
	if (!is_active_player(player) || !is_wait_active1())
	{
		msg->send_failed_reason(GG_ERROR_INVALID_STATUS);
		return;
	}

	const GG_ApplyDigCardRequest& pbrequest = msg->message<GG_ApplyDigCardRequest>();
	assert(!m_bottom_cards.empty() && !m_pokercard_pool.empty());
	PokerCard card; 
	if (pbrequest.digtype() == DIG_BOTTOM)
	{
		card = m_bottom_cards.front();
		m_bottom_cards.erase(m_bottom_cards.begin());
	}
	else
	{
		//如果是赖子牌，并且不是第一张，那么不能摸
		card = m_pokercard_pool.back();
		if (CombinationInstance::get_const_instance().is_joker(card, m_joker_card.m_point) && !m_is_first_pool_card)
		{
			DEBUG_LOG << "dig joker card:" << card.to_string() << " is not first pool card:" << (m_is_first_pool_card);
			return;
		}
		m_pokercard_pool.pop_back();
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	m_is_first_pool_card = false;
	m_seats[m_active_seatnum].dig_card(card);
	GG_PlayerDigCardRequest request;
	request.set_userid(player->get_user_id());
	card.to_msg_card(request.mutable_card());
	request.set_digtype(pbrequest.digtype()) ;
	DEBUG_LOG << "user(" << request.userid() << ") dig card bottom_card_count:" << m_bottom_cards.size() << " dig_card:" << card.to_string();
	m_room->broadcast_player_digcard(player, request);
	m_wait_begin = time(NULL);
	if (player->is_player_trusteed_on())
	{
		wait_for_round(GAME_ACTIVE2, 1000);
	}
	else
	{
		wait_for_round(GAME_ACTIVE2, m_room->get_game_wait_interval().m_time_active2 + 1500);
	} 

	DEBUG_LOG << "after dig card cur bottom count:" << (m_bottom_cards.size());
	m_game_log_record.dig_card(request);
	m_game_log_record.active2_player(m_active_seatnum, player->get_user_id());
}

void GameDesk::broadcast_post_card(const PLAYER_PTR&  player, const PokerCard& card)
{
	PlayerPostCardRequest request;
	request.set_userid(player->get_user_id());
	card.to_msg_card(request.mutable_card());
	m_room->broadcast_message(GG_CMD_PLAYER_POST_CARD, request);
	m_game_log_record.post_card(request);
}

void GameDesk::post_default_card()
{
	const PLAYER_PTR&  player = m_seats[m_active_seatnum].get_seated_player();
	PokerCard card = m_seats[m_active_seatnum].post_default_card();
	m_pokercard_pool.push_back(card);
	broadcast_post_card(player, card);
	if (m_bottom_cards.size() == 1)
	{
		step_shuffle_card();
		wait_for_round(GAME_SHUFFLE_CARD, m_room->get_game_wait_interval().m_time_shuffle_card + m_room->get_game_wait_interval().m_time_post_card);
		//重新洗牌
	}
	else
	{
		active_player(m_room->get_game_wait_interval().m_time_post_card);
	}
}

bool GameDesk::check_card(const int point, const PokerCard& card,  PokerCard &pool_card)
{
	return false;
}

void GameDesk::player_post_card(const PLAYER_PTR&  player, const MESSAGE_PTR & msg)
{
	const GG_ApplyPostCardRequest& pbrequest = msg->message<GG_ApplyPostCardRequest>();
	GG_ApplyPostCardResponse response;
	CopyCardToMsg(player->get_seat()->get_hand_cards_total(), response.mutable_handcards());
	CopyCardToMsg(m_pokercard_pool, response.mutable_poolcards());
	if (!is_active_player(player) || !is_wait_active2())
	{
		msg->send_response(GG_ERROR_INVALID_STATUS, response);
		return;
	}
	PokerCard card;
	card.parse_msg_card(pbrequest.card());
	if (!m_seats[m_active_seatnum].discard_card(card))
	{
		msg->send_response(GG_ERROR_PARAMETER_INVALID, response);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	m_pokercard_pool.push_back(card);
	broadcast_post_card(player, card);
	if (m_bottom_cards.size() == 1)
	{
		step_shuffle_card();
		wait_for_round(GAME_SHUFFLE_CARD, m_room->get_game_wait_interval().m_time_shuffle_card + m_room->get_game_wait_interval().m_time_post_card);
		//重新洗牌
	}
	else
	{
		active_player(m_room->get_game_wait_interval().m_time_post_card);
	}
	notify_room_will_over(pbrequest.appscore());
	LOG_DEBUG("player_post_card cur_score:" + toString(pbrequest.appscore()));
}

void GameDesk::step_gameover_distrbute(void)
{
	//每个玩家结算， 并且投入奖池
	GG_GameOverRequest request;
	TY_INT8 win;
	caculate_over_account(request);
	//广播结算
	m_room->broadcast_message(GG_CMD_GAME_OVER, request);
	submit_game_result();
	//等待结算动画
	m_wait_begin = time(NULL);
	if (m_gameover_type == DROP_OVER)
	{
		//多等个drop动画
		wait_for_round(GAME_DISTRIBUTE, m_room->get_game_wait_interval().m_time_distribute 
			+ m_room->get_game_wait_interval().m_time_account_animat + 3000);
		
	}
	else
	{
		wait_for_round(GAME_DISTRIBUTE, m_room->get_game_wait_interval().m_time_distribute + m_room->get_game_wait_interval().m_time_account_animat);
		
	}
	
	set_ingame_player_over_status();
	clear_desk();
}


void GameDesk::get_seated_player_chips(google::protobuf::RepeatedPtrField<PlayerChips>* playChips)
{

	for (std::vector<GameSeat>::iterator it = m_seats.begin(); it != m_seats.end(); ++it)
	{
		if (!it->is_in_game())
		{
			continue;
		}
		PlayerChips* oneChips = playChips->Add();
		oneChips->set_userid(it->get_ingame_player_id());
		oneChips->set_currentchips(it->get_seated_player()->get_chips());
	}
}

void GameDesk::deduct_desk_tips(void)
{
	
	//m_game_log_record.set_game_info();
}

bool GameDesk::is_active_player(const PLAYER_PTR&  player)
{
	if (m_active_seatnum == -1)
	{
		return false;
	}
	return player->get_seat_number() == m_active_seatnum;
}


void GameDesk::player_declare(const PLAYER_PTR&  player, const MESSAGE_PTR& msg)
{
	TRACE_FUNCATION();
	if (!player->is_seated_player() || !player->get_seat()->is_in_game() || !is_wait_active2() || !is_active_player(player))
	{
		WARN_LOG << "user(" << player->get_user_id() << ") declare error status not valid";
		msg->send_failed_reason(GG_ERROR_PARAMETER_INVALID);
		return;
	}
	const GG_ApplyDeclareRequest& pbrequest = msg->message<GG_ApplyDeclareRequest>();
	if (!player->get_seat()->check_declare(pbrequest))
	{
		WARN_LOG << "user(" << player->get_user_id() << ") declare error check_declare failed";
		msg->send_failed_reason(GG_ERROR_PARAMETER_INVALID);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	
	GG_PlayerDeclareRequest request;
	request.set_userid(player->get_user_id());
	request.mutable_declarecard()->CopyFrom(pbrequest.declarecard());
	request.mutable_cardgroup()->CopyFrom(pbrequest.cardgroup());
	m_room->broadcast_message(GG_CMD_PLAYER_DECLARE, request);
	
	//进入declare rand;
	m_wait_begin = time(NULL);
	wait_for_round(GAME_DECLARE, m_room->get_game_wait_interval().m_time_declare + 
		m_room->get_game_wait_interval().m_time_declare_animat + 3000);

	m_apply_declare_seatnum = player->get_seat_number();
	notify_room_status(GG_FlushRoomStausRequest::ROOM_IDLE);
	m_gameover_type = NORMAL_OVER;
	m_game_log_record.player_declare(request);
}

void GameDesk::player_lose_declare(const PLAYER_PTR&  player, const MESSAGE_PTR& msg)
{
	if (!player->is_seated_player() || !player->get_seat()->is_in_game() || !is_declareing())
	{
		WARN_LOG << "user(" << player->get_user_id() << ") declare error status not valid";
		msg->send_failed_reason(GG_ERROR_PARAMETER_INVALID);
		return;
	}
	int score = 0;
	const GG_ApplyLoseDeclareRequest& pbrequest = msg->message<GG_ApplyLoseDeclareRequest>();
	if (!player->get_seat()->check_lose_declare(pbrequest, score))
	{
		WARN_LOG << "user(" << player->get_user_id() << ") declare error check_declare failed";
		msg->send_failed_reason(GG_ERROR_PARAMETER_INVALID);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	TY_INT64 lose_chips = m_room->get_room_info().m_small_blind * player->get_seat()->get_score();
	TY_INT64 real_chips = player->get_seat()->match_over_account(-lose_chips);
	GG_PlayerLoseDeclareRequest request;
	request.set_userid(player->get_user_id());
	request.mutable_cardgroup()->CopyFrom(pbrequest.cardgroup());
	request.set_score(score);
	request.set_chips(real_chips);
	m_room->broadcast_message(GG_CMD_PLAYER_LOSE_DECLARE, request);
	m_chips_pool += real_chips;
	//如果所有的人都declare， 那么进入结算状态
	if (get_ingame_count() == 1)
	{
		step_gameover_distrbute();
	}
	m_game_log_record.loser_declare(request);
}

void GameDesk::default_decalre(const PLAYER_PTR&  player)
{
	TRACE_FUNCATION();
	int score = 0;
	std::vector<CardGroup> cg;
	player->get_seat()->default_declare(score, cg);
	TY_INT64 lose_chips = m_room->get_room_info().m_small_blind * player->get_seat()->get_score();
	TY_INT64 real_chips = player->get_seat()->match_over_account(-lose_chips);
	MAKE_PB(GG_PlayerLoseDeclareRequest, request);
	request->set_userid(player->get_user_id());
	copy_group_to_msg(cg, request->mutable_cardgroup());
	request->set_score(score);
	request->set_chips(real_chips);
	m_chips_pool += real_chips;
	m_room->broadcast_message(GG_CMD_PLAYER_LOSE_DECLARE, request);
	//如果所有的人都declare， 那么进入结算状态
	if (get_ingame_count() == 1)
	{
		step_gameover_distrbute();
	}
}

void GameDesk::player_trusteeoff(const PLAYER_PTR&  player, const MESSAGE_PTR& msg)
{
	const GG_ApplyTrusteeOffRequest& pbrequest = msg->message<GG_ApplyTrusteeOffRequest>();
	if (!player->is_seated_player())
	{
		return;
	}

	if (is_wait_active() && is_active_player(player) && player->is_player_trusteed_on())
	{
		time_t tnow = time(NULL);
		//player->m_isPlayerCancelTrustee = true;
		int lefttime = 0;
		if (is_wait_active1())
		{
			lefttime = m_room->get_game_wait_interval().m_time_active1;
		}
		else
		{
			lefttime = m_room->get_game_wait_interval().m_time_active2;
		}
		lefttime = lefttime/1000 - (tnow - m_wait_begin);
		if (lefttime <= 0)
		{
			lefttime = 0;
		}
		if (is_wait_active1())
		{
			wait_for_round(GAME_ACTIVE1, lefttime * 1000 + 1500));
		}
		else
		{
			wait_for_round(GAME_ACTIVE2, lefttime * 1000 + 1500);
		}
	}

	player->set_trustee_off();
}

TY_INT8 GameDesk::get_last_ingame_seatnum()
{
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (iter->is_seated() && iter->is_in_game())
		{
			return iter->get_seat_number();
		}
	}
	return -1;
}

void GameDesk::step_player_drop(const PLAYER_PTR&  player)
{
	GG_PlayerDropRequest request;
	
	if (is_wait_active2() && is_active_player(player))
	{
		player->get_seat()->player_drop(true);
		player->get_seat()->get_dig_card().to_msg_card(request.mutable_dropcard());
	}
	else
	{
		player->get_seat()->player_drop(false);
		PokerCard card;
		card.to_msg_card(request.mutable_dropcard());
	}

	request.set_dropscore(player->get_seat()->get_score());
	TY_INT64 real_chips = player->get_seat()->match_over_account(-m_room->get_room_info().m_small_blind * player->get_seat()->get_score());
	request.set_dropchips(real_chips);
	request.set_userid(player->get_user_id());
	
	m_chips_pool += real_chips;
	m_room->broadcast_message(GG_CMD_PLAYER_DROP, request);

	m_game_log_record.player_drop(player->get_user_id(), player->get_seat_number(), request.dropscore(), request.dropchips());

	int undropCount = get_ingame_count();
	if (undropCount == 1)
	{
		m_gameover_type = DROP_OVER;
		m_apply_declare_seatnum = get_last_ingame_seatnum();
		//算出最优牌型
		m_seats[m_apply_declare_seatnum].drop_win();
		assert(m_apply_declare_seatnum >= 0);
		notify_room_status(GG_FlushRoomStausRequest::ROOM_IDLE);
		step_gameover_distrbute();
		return;
	}
	  
	if (is_wait_active() && is_active_player(player))
	{
		active_player(m_room->get_game_wait_interval().m_time_drop);
	}
}

void GameDesk::player_drop(const PLAYER_PTR&  player, const MESSAGE_PTR&msg)
{
	if (is_game_over() || is_game_declare() || is_dealer_card())
	{
		msg->send_failed_reason(GG_ERROR_INVALID_STATUS);
		return;
	}
	if (!player->is_seated_player() || !player->get_seat()->is_in_game() || player->get_seat()->is_drop())
	{
		msg->send_failed_reason(GG_ERROR_INVALID_STATUS);
		return;
	}
	msg->send_failed_reason(GG_ERROR_SUCCESS);
	step_player_drop(player);
}


void GameDesk::player_close_account_box(const PLAYER_PTR&  player)
{
	if (!player->is_seated_player())
	{
		return;
	}

	if (!is_distribut())
	{
		return;
	}

	if (!player->is_player_playing())
	{
		return;
	}

	player->set_status((player->get_status() & ~Player::PLAYER_MASK) | Player::PLAYER_UNREADY);
	GG_PlayerCloseAccountBoxRequest req;
	req.set_userid(player->get_user_id());
	m_room->broadcast_message(GG_CMD_PLAYER_CLOSE_ACCOUNT_BOX, req);
	if (!is_all_player_over_account())
	{
		return;
	}

	//弹框倒计时提前结束
	ready_for_next_game();
}

bool GameDesk::is_all_player_over_account()
{
	for (auto iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (!iter->is_seated())
		{
			continue;
		}
		if (iter->get_seated_player()->is_player_playing())
		{
			return false;
		}
	}
	return true;
}

void GameDesk::deduct_draw_tips()
{
	TY_INT64 one_tips = m_room->get_room_info().m_draw_tips;
	if (one_tips == 0)
	{
		return;
	}
	for (std::vector<GameSeat>::iterator iter = m_seats.begin(); iter != m_seats.end(); ++iter)
	{
		if (iter->is_in_game())
		{
			const PLAYER_PTR&  player = iter->get_seated_player();
			TY_INT64 realchips = 0;
			player->bet_chips(one_tips, PlayerChipsChangeRequest::game_tips, realchips);
			m_game_log_record.deduct_desk_tips(player->get_user_id(), realchips);
			DEBUG_LOG << "user(" << player->get_user_id() << ") draw tips:" << realchips;
		}
	}
}
