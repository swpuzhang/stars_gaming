#include "CardDealer.h"
#include <stdlib.h>
#include <set>
#include <assert.h>
#include <time.h>
#include <algorithm>

//////////////////////////////////////////////////////////////////////////
void PokerCard::to_msg_card(MsgCard* card) const
{
	card->set_point(m_point);
	card->set_color(m_color);
}

void PokerCard::parse_msg_card(const MsgCard& card)
{
	m_point = card.point();
	m_color = card.color();
}

std::string PokerCard::to_string(void) const
{
	std::string str(std::to_string((int)m_color) + ":" + std::to_string((int)m_point));
	return str;
}

void PokerCard::parse_string(const std::string& v)
{
	std::string::size_type pos = v.find(':');
	if (std::string::npos == pos)
	{
		return;
	}
	std::string str_point_up = v.substr(0, pos);
	std::string str_point_down = v.substr(pos + 1);
	m_color = (char)atoi(str_point_up.c_str());
	m_point = (char)atoi(str_point_down.c_str());
}

short PokerCard::to_db_card(void) const
{
	unsigned short db_card = m_color;
	db_card <<= 8;
	db_card += m_point;
	return db_card;
}

void PokerCard::parse_db_card(unsigned short v)
{
	m_point = v & 0x00FF;
	v >>= 8;
	m_color = (unsigned char)v;
}

int PokerCard::get_score(int joker_point) const
{
	if (m_point >= P2 && m_point <= A && m_point != joker_point)
	{
		return m_point > 10 ? 10 : m_point;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
void CardGroup::to_msg_group(PB_CardGroup* group) const
{
	group->set_type(m_type);
	CopyCardToMsg(m_cards, group->mutable_cards());
}

void CardGroup::parse_msg_group(const PB_CardGroup& group)
{
	m_type = group.type();
	for (auto it = group.cards().begin(); it != group.cards().end(); ++it)
	{
		PokerCard card;
		card.parse_msg_card(*it);
		m_cards.push_back(card);
	}
}

//////////////////////////////////////////////////////////////////////////

CardDealer::CardDealer(void)
{
	init_cards();
}


CardDealer::~CardDealer(void)
{
}

void CardDealer::init_rand_seed(void)
{
	srand((unsigned int)::time(NULL));
}

void CardDealer::deal_card(std::vector<PokerCard> &lstCard, size_t count)
{
	rand_deal_card(lstCard, count);
}


void CardDealer::rand_deal_card(std::vector<PokerCard> &lstCard, size_t count)
{
	srand((TY_UINT32)time(NULL));
    assert(count <= CARD_COUNT);
    rand_rank_cards();

    std::vector<PokerCard> cards = m_cards;
    size_t get_count = 0;
    while (get_count < count)
    {
        int index = rand() % cards.size();
        lstCard.push_back(cards[index]);
        get_count++;
        if (cards.size() > 1)
        {
            cards[index] = cards.back();
            cards.pop_back();
        }
    }
}

void CardDealer::init_cards(void)
{
    m_cards.resize(CARD_COUNT);
	int index = 0;
	for (int i = 0; i < PokerCard::diamonds; i++)
	{
		for (int j = 0; j < PokerCard::A - 1; j++)
		{
			m_cards[index].m_color = i + 1;
			m_cards[index].m_point = j + 2;
			++index;
			m_cards[index].m_color = i + 1;
			m_cards[index].m_point = j + 2;
			++index;
		}
	}
	m_cards[index].m_color = 0;
	m_cards[index].m_point = PokerCard::JOKER1;
	++index;
	m_cards[index].m_color = 0;
	m_cards[index].m_point = PokerCard::JOKER1;
	++index;
	m_cards[index].m_color = 0;
	m_cards[index].m_point = PokerCard::JOKER2;
	++index;
	m_cards[index].m_color = 0;
	m_cards[index].m_point = PokerCard::JOKER2;
	++index;
	m_all_card = m_cards;
	for (int i = 0; i < 1000; ++i)
	{
		rand_rank_cards();
	}
}



void CardDealer::rand_rank_cards(void)
{
    assert(m_cards.size() == CARD_COUNT);
    std::vector<PokerCard> cards = m_cards;
    m_cards.clear();
    size_t get_count = 0;
    while (get_count < CARD_COUNT)
    {
        int index = rand() % cards.size();
        m_cards.push_back(cards[index]);
        get_count++;
        if (cards.size() > 1)
        {
            cards[index] = cards.back();
            cards.pop_back();
        }
    }
}


