#ifndef GG_PokerCard_DEALER_H__
#define GG_PokerCard_DEALER_H__

#include <vector>
#include "libmessage/Message.h"
#include "GameMsg.pb.h"
#include "liblog/Log.h"

using namespace GameMsg;


//一张扑克牌
class PokerCard
{
public:
	//牌点数
	enum PokerColor
	{
		//黑桃
		spades = 1,
		//红桃
		hearts,
		//梅花
		clubs,
		//方块
		diamonds
	};

	enum PokerPoint
	{
		P2 = 2,
		P3,
		P4,
		P5,
		P6,
		P7,
		P8,
		P9,
		P10,
		J,
		Q,
		K,
		A,
		JOKER1,
		JOKER2,
	};
public:
	PokerCard(void) : m_color(0), m_point(0) {};

	void to_msg_card(MsgCard* card) const;
	void parse_msg_card(const MsgCard&);
	std::string to_string(void) const;
	void parse_string(const std::string& v);
	short to_db_card(void) const;
	void parse_db_card(unsigned short v);
	bool is_valid_card(void) const
	{
		DEBUG_LOG << "point:" << (int)m_point << " color:" << (int)m_color;
		if (m_point >= JOKER1)
		{
			return m_point <= JOKER2;
		}
		return (m_color >= spades && m_color <= diamonds && m_point >= P2 && m_point <= A);
	};
	int get_score(int joker_point) const;
	bool operator==(const PokerCard& other) const { return m_color == other.m_color && m_point == other.m_point; };

	bool operator!=(const PokerCard& other) const { return !operator==(other); };

	bool operator<(const PokerCard& other) const { return m_point != other.m_point ? m_point < other.m_point : m_color < other.m_color; };

public:
	TY_UINT8 m_color;
	TY_UINT8 m_point;
};


class CardGroup
{
public:
	enum
	{
		GROUP_INVALID, //散牌
		GROUP_STAIGHT, //同花顺
		GROUP_THREE_KIND, //三条
	};
	void to_msg_group(PB_CardGroup* group) const;
	void parse_msg_group(const PB_CardGroup&);
	CardGroup() : m_type(0) {}
public:
	int m_type;
	std::vector<PokerCard> m_cards;
};

//牌序列
typedef std::vector<PokerCard> CARD_SEQUNCE;


class CardDealer
{
public:
	enum {JOKER_CARD = 9,  HAND_CARD_COUNT = 13, CARD_COUNT = 108 };
public:
	CardDealer(void);
	virtual ~CardDealer(void);

	virtual void init_rand_seed(void);

	virtual void deal_card(std::vector<PokerCard> &lstCard, size_t count);

	void get_all_card(std::vector<PokerCard>& card) { card = m_all_card; };

protected:
	void rand_deal_card(std::vector<PokerCard> &lstCard, size_t count);

private:
	void init_cards(void);

    void rand_rank_cards(void);

protected:
	std::vector<PokerCard> m_cards;
	std::vector<PokerCard> m_all_card;
};

template<typename T>
void CopyCardToMsg(const T& handCard,
	google::protobuf::RepeatedPtrField<MsgCard>* pbCard)
{
	for (typename T::const_iterator it = handCard.begin(); it != handCard.end(); ++it)
	{
		MsgCard *oneCard = pbCard->Add();
		it->to_msg_card(oneCard);
	}
}

template<typename T>
void copy_group_to_msg(const T& group,
	google::protobuf::RepeatedPtrField<PB_CardGroup>* pbGroup)
{
	for (typename T::const_iterator it = group.begin(); it != group.end(); ++it)
	{
		PB_CardGroup *oneGroup = pbGroup->Add();
		it->to_msg_group(oneGroup);
	}
}

template<typename T>
void ParseMsgGroup(T& groups,
	const google::protobuf::RepeatedPtrField<PB_CardGroup>& pbGroup)
{
	for (auto it = pbGroup.begin(); it != pbGroup.end(); ++it)
	{
		CardGroup g;
		g.parse_msg_group(*it);
		groups.push_back(g);
	}
}

#endif // GG_PokerCard_DEALER_H__

