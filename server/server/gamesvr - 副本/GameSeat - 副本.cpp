#include "GameSeat.h"
#include "Player.h"
#include "GameDesk.h"
#include "GameRoom.h"
#include "GG_GameLogRecord.h"
#include "GG_Money_Proxy.h"
#include "Combination.h"
#include "liblog/Log.h"
#include <assert.h>


GameSeat::GameSeat(void)
	: m_desk(NULL), m_player(NULL), m_seat_number(-1), m_seat_stauts(0)
	, m_ingame_player_id(0), m_match_over_account_chips(0), 
	m_score(0), m_card_score(0)
{
	LOG_TRACE("seat:" + toString((long)this));
}


GameSeat::~GameSeat(void)
{
}

void GameSeat::ready_for_game(void)
{
	TRACE_FUNCATION();
	m_ingame_player_id = 0;
	m_score = 20;
	m_card_score = 0;
	if (is_seated())
	{
		seat_status(seated | ingame | cur_join);
		m_ingame_player_id = m_player->get_user_id();
        m_player->ready_for_game();
	}
	else
	{
		seat_status(0);
	}
   
    m_match_over_account_chips = 0;
    m_handcards_total.clear();
}

void GameSeat::set_game_desk(GameDesk *desk)
{
	m_desk = desk;
}

void GameSeat::set_seat_number(int number)
{
	m_seat_number = number;
}

bool GameSeat::is_seated(void) const
{
	return m_seat_stauts & GameSeat::seated;
}

bool GameSeat::is_in_game(void) const
{
	return ((m_seat_stauts & GameSeat::ingame));
}

int GameSeat::seat_status(void)
{
	return m_seat_stauts;
}

int GameSeat::seat_status(int new_status)
{
	INFO_LOG << "room(" << (m_desk->get_room()->get_room().m_area_id) + ") seat(" + toString(m_seat_number) + ") status(" + get_status_name(m_seat_stauts) + " -> " + get_status_name(new_status) + ")");
	return m_seat_stauts = new_status;
}

PLAYER_PTR  GameSeat::get_seated_player(void) const
{
	return m_player;
}

void GameSeat::player_stand_up(void)
{
	LOG_TRACE("room(" + toString(m_desk->get_room()->get_room_info().m_area_id) + ") seat(" + toString(m_seat_number) + ") player(" + toString(m_player->get_user_id()) + ") stand up");

	if (!m_desk->is_game_over())
	{
		seat_status((seat_status() & ~GameSeat::ingame) & ~GameSeat::seated);
	}
	
	else
	{
		seat_status(0);
	}

	m_desk->get_game_log_record()->player_stand_up(m_player->get_user_id(), m_seat_number);
	m_player->stand_up();
	m_player = NULL;
	m_handcards_total.clear();
	
}

void GameSeat::game_over()
{
	seat_status((seat_status() & ~GameSeat::ingame) & ~GameSeat::escape);
}

void GameSeat::player_sit_down(PLAYER_PTR player)
{
	LOG_TRACE("room(" + toString(m_desk->get_room()->get_room_info().m_area_id) + ") seat(" + toString(m_seat_number) + ") player(" + toString(player->get_user_id()) +") sit down");
	m_player = player;
	seat_status(seat_status() | GameSeat::seated);
	player->set_game_desk(m_desk);
	player->sit_down(this);
	m_desk->get_game_log_record()->player_sit_down(m_player->get_user_id(), m_seat_number);
}


int GameSeat::get_seat_number(void) const
{
	return m_seat_number;
}

bool GameSeat::is_winner(void) const
{
	return m_seat_stauts & GameSeat::win;
}


void GameSeat::deal_cards(const std::vector<PokerCard> &hand_cards)
{
    m_handcards_total = hand_cards;
	std::sort(m_handcards_total.begin(), m_handcards_total.end());
    //m_player->set_status(Player::PLAYER_SORT | (m_player->get_status()  & ~Player::PLAYER_MASK));
}

const std::vector<PokerCard>& GameSeat::get_hand_cards_total(void)
{
	return m_handcards_total;
}


std::string GameSeat::get_status_name(int status) const
{
	std::string status_name;
	if (status & seated)
	{
		status_name += status_name.empty() ? "" : "|";
		status_name += "seated";
	}
	if (status & ingame)
	{
		status_name += status_name.empty() ? "" : "|";
		status_name += "ingame";
	}
	if (status & win)
	{
		status_name += status_name.empty() ? "" : "|";
		status_name += "win";
	}
  
	if (status & cur_join)
	{
		status_name += status_name.empty() ? "" : "|";
		status_name += "cur_join";
	}

	if (status & escape)
	{
		status_name += status_name.empty() ? "" : "|";
		status_name += "escape";
	}

	if (status_name.empty())
	{
		status_name = "idle";
	}
	return status_name;
}

TY_INT64 GameSeat::get_match_chips_account(void)
{
    return m_match_over_account_chips;
}


TY_INT64 GameSeat::match_over_account(TY_INT64 chips)
{
	TY_INT64 real_count = 0;
	TY_INT64 all_money = 0;
	if (chips <= 0)
	{
		LOG_DEBUG("player(" + toString(m_player->get_user_id()) 
			+ ") match_over_account deal_loser_account:" + toString(chips));
		m_player->deal_loser_account(-chips, real_count, all_money, 
			m_desk->get_room()->get_room_info().m_area_id, m_desk->get_game_log_record()->get_game_id());
		m_match_over_account_chips -= real_count;
	}
	else
	{
		seat_status(m_seat_stauts | win);
		m_player->carry_add(chips);
		real_count = chips;
		m_match_over_account_chips += real_count;
		send_match_over_account();
		all_money = get_seated_player()->get_user_money();
	}

	LOG_DEBUG("player(" + toString(m_player->get_user_id()) + ") match_over_account: chips:" 
		+ toString(chips) + " real_count:" + toString(real_count));
	return real_count;
}


void GameSeat::send_match_over_account(void)
{
	
    GG_MatchOverAccountRequest request;
	request.set_gameid(m_desk->get_game_log_record()->get_game_id());
	request.set_roomid(m_desk->get_room()->get_room_info().m_area_id);
	request.set_userid(m_player->get_user_id());
	LOG_DEBUG("player(" + toString(m_player->get_user_id()) + ") seatnum(" + toString(m_seat_number)
		+ ") send_match_over_account before cur_carry:" + toString(m_player->get_carry())
		+ " small blind:" + toString(m_desk->get_room()->get_room_info().m_small_blind));
 
	TY_INT64 cur_carray = m_player->get_carry();
	TY_INT64 buy_carry = m_desk->get_sitdown_buyin_count();
	TY_INT64 temp_count = cur_carray - buy_carry;
	if (temp_count <= 0)
	{
		request.set_needaddtoactive(0);
		request.set_curcarrychips(cur_carray);
	}
	else
	{
		m_player->set_carry(buy_carry);
		request.set_needaddtoactive(temp_count);
		request.set_curcarrychips(m_player->get_carry());
	}

	MQService::instance()->send_mqrequest(GG_CMD_MATCH_OVER_ACCOUNT, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request);
}


//在比牌的时候判断， 其他时候没有逃跑这个概念
bool GameSeat::is_player_escape()
{
	if (m_seat_stauts & GameSeat::escape)
	{
		LOG_DEBUG("player (" + toString(m_ingame_player_id) + ") is escape:true");
		return true;
	}
	return false;
}

//跟牌和出牌都会从手牌里面丢弃一些牌， 检查将要丢弃的牌是否存在手牌，是否合法
bool GameSeat::discard_card(const PokerCard& card)
{	
	std::vector<PokerCard>::iterator itFind = std::find(m_handcards_total.begin(), m_handcards_total.end(), card);
	if (itFind == m_handcards_total.end())
	{
		return false;
	}
	m_handcards_total.erase(itFind);
	return true;
}

PokerCard GameSeat::post_default_card()
{
	TRACE_FUNCATION();
	//PokerCard card;
	//int cur_score = 0;
	//CombinationInstance::get_const_instance().post_best_card(m_handcards_total, m_desk->get_joker_card().m_point, card, cur_score);
	CARD_SEQUNCE::iterator it = std::find(m_handcards_total.begin(), m_handcards_total.end(), m_dig_card);
	assert(it != m_handcards_total.end());
	m_handcards_total.erase(it);
	//CombinationInstance::get_const_instance().
	//m_desk->notify_room_will_over(cur_score);
	//LOG_DEBUG("post_default_card cur_score:" + toString(cur_score));
	return m_dig_card;
}


TY_INT64 GameSeat::game_over_account(const TY_INT64 small_blind)
{
	int accout_size = m_handcards_total.size() - 1;
	if (accout_size <= 0)
		return 0;
	return accout_size * small_blind;
}


void GameSeat::reset_seat()
{
	m_match_over_account_chips = 0;
	m_handcards_total.clear();
	m_dig_card = PokerCard();
	m_card_group.clear();
}

void GameSeat::dig_card(const PokerCard &card)
{
	m_score = 40;
	m_dig_card = card;
	m_handcards_total.push_back(card);
	std::sort(m_handcards_total.begin(), m_handcards_total.end());
}

void GameSeat::borning_over()
{
	m_score = 0;
	CombinationInstance::get_const_instance().default_declare(m_handcards_total, m_desk->get_joker_card().m_point, m_card_score, m_card_group);
	DEBUG_LOG << "user(" << m_ingame_player_id << ")" << " borning_over score:" << m_card_score << " card_group:" << m_card_group.size();
}

void GameSeat::drop_win()
{
	m_score = 0;
	CombinationInstance::get_const_instance().default_declare(m_handcards_total, m_desk->get_joker_card().m_point, m_card_score, m_card_group);
	DEBUG_LOG << "user(" << m_ingame_player_id << ")" << " drop_win score:" << m_card_score << " card_group:" << m_card_group.size();
}

void GameSeat::player_drop(bool drop_card)
{
	seat_status((seat_status() & ~GameSeat::ingame) | drop);
	m_player->set_status((m_player->get_status() & ~Player::PLAYER_MASK) | Player::PLAYER_DROP);
	if (drop_card)
	{
		CARD_SEQUNCE::iterator it = std::find(m_handcards_total.begin(), m_handcards_total.end(), m_dig_card);
		assert(it != m_handcards_total.end());
		ERROR_LOG << ("user(") << m_ingame_player_id << ") drop card:" << m_dig_card.to_string() << " error";	
		m_handcards_total.erase(it);
	}
	CombinationInstance::get_const_instance().default_declare(m_handcards_total, m_desk->get_joker_card().m_point, m_card_score, m_card_group);
	DEBUG_LOG << "user(" << m_ingame_player_id << ")" << "drop score:" << m_card_score << " card_group:" << m_card_group.size();
}

bool GameSeat::check_declare(const GG_ApplyDeclareRequest& msg)
{
	TRACE_FUNCATION();
	DEBUG_LOG << "hand_card:" << CombinationInstance::get_const_instance().cards_to_string(m_handcards_total);
	PokerCard card;
	card.parse_msg_card(msg.declarecard());
	DEBUG_LOG << "declare_card:" << card.to_string();
	std::stringstream strstr;
	strstr << "card_group:";
	std::vector<CardGroup> groups;
	ParseMsgGroup(groups, msg.cardgroup());
	for (std::vector<CardGroup>::const_iterator it = groups.begin(); it != groups.end(); ++it)
	{
		strstr << " card_type:" << it->m_type << " cards:" << CombinationInstance::get_const_instance().cards_to_string(it->m_cards);
	}
	DEBUG_LOG << strstr.str();
	
	PokerCard deccard = card;
	CARD_SEQUNCE::iterator it = std::find(m_handcards_total.begin(), m_handcards_total.end(), deccard);
	if (it == m_handcards_total.end())
	{
		ERROR_LOG << ("user(") << msg.userid() << ") check_declare card:" << deccard.to_string() << " error";
		return false;
	}
	CARD_SEQUNCE decsequnce;
	for (std::vector<CardGroup>::const_iterator it = groups.begin(); it != groups.end(); ++it)
	{
		std::copy(it->m_cards.begin(), it->m_cards.end(), std::back_inserter(decsequnce));
	}
	decsequnce.push_back(deccard);
	std::sort(m_handcards_total.begin(), m_handcards_total.end());
	std::sort(decsequnce.begin(), decsequnce.end());
	if (decsequnce != m_handcards_total)
	{
		ERROR_LOG << ("user(") << msg.userid() << ") check_declare error";
		return false;
	}

	//检查是否declare
	if (!CombinationInstance::get_const_instance().is_groups_declare(groups, m_desk->get_joker_card().m_point))
	{
		ERROR_LOG << ("user(") << msg.userid() << ") check_declare error";
		return false;
	}
	m_score = 0;
	m_card_score = 0;
	m_card_group = groups;
	seat_status(seat_status()| declared);
	m_player->set_status((m_player->get_status() & ~Player::PLAYER_MASK) | Player::PLAYER_DECLARE);
	return true;
}

bool GameSeat::check_lose_declare(const GG_ApplyLoseDeclareRequest& msg, int& score)
{
	
	std::vector<CardGroup> groups;
	ParseMsgGroup(groups, msg.cardgroup());

	CARD_SEQUNCE decsequnce;
	for (std::vector<CardGroup>::const_iterator it = groups.begin(); it != groups.end(); ++it)
	{
		std::copy(it->m_cards.begin(), it->m_cards.end(), std::back_inserter(decsequnce));
	}
	std::sort(m_handcards_total.begin(), m_handcards_total.end());
	std::sort(decsequnce.begin(), decsequnce.end());
	if (decsequnce != m_handcards_total)
	{
		ERROR_LOG << ("user(") << msg.userid() << ") check_lose_declare error";
		return false;
	}

	if (!CombinationInstance::get_const_instance().check_lose_declare(groups, m_desk->get_joker_card().m_point, score))
	{
		return false;
	}

	//算出score;
	if (score == 0)
	{
		m_card_score = 0;
		m_score = 1;
	}
	else if (score > 80)
	{
		m_card_score = score;
		score = 80;
		m_score = 80;
	}
	else
	{
		m_score = score;
		m_card_score = m_score;
	}
	
	m_card_group = groups;
	seat_status((seat_status() & ~GameSeat::ingame) | declared);
	m_player->set_status((m_player->get_status() & ~Player::PLAYER_MASK) | Player::PLAYER_DECLARE);
	return true;
}

void GameSeat::default_declare(int& score, std::vector<CardGroup>& cardGroup)
{
	TRACE_FUNCATION();
	CombinationInstance::get_const_instance().default_declare(m_handcards_total, m_desk->get_joker_card().m_point, score, cardGroup);
	//算出score;
	if (score == 0)
	{
		m_card_score = 0;
		m_score = 1;
	}
	else if (score > 80)
	{
		m_card_score = score;
		score = 80;
		m_score = 80;
	}
	else
	{
		m_score = score;
		m_card_score = m_score;
	}
	
	m_card_group = cardGroup;
	seat_status((seat_status() & ~GameSeat::ingame) | declared);
	m_player->set_status((m_player->get_status() & ~Player::PLAYER_MASK) | Player::PLAYER_DECLARE);
}