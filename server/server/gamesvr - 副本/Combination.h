#pragma once

#include <list>
#include <chrono>
#include <vector>
#include <boost/serialization/singleton.hpp>
#include "CardDealer.h"


typedef std::vector<std::vector<PokerCard> > CARD_COMBINE;
typedef std::vector<std::vector<int> > VEC_INDEX_COMBINE;
static const  int MAX_CARD_COUNT = 14;
class CombinationInfo
{
public:
	CombinationInfo() : m_score(0), m_is_has_firtlife(false), m_is_has_secondelife(false) {}
	int m_score;
	std::vector<CardGroup> m_card_group;
	std::vector<PokerCard> m_valid_cards;
	bool m_is_has_firtlife;
	bool m_is_has_secondelife;
};

class CardRelation
{
public:
	CardRelation() : m_relate_degree(0) {}
	PokerCard m_card;
	int m_relate_degree; //关联度
};

class Combination
{
public:
	Combination() : m_joker_point(0) {}

	template <typename T>
	int caculate_card_score(T& cards, int joker_point)
	{
		int score = 0;
		for (typename T::const_iterator it = cards.begin(); it != cards.end(); ++it)
		{
			score += it->get_score(joker_point);
		}
		return score;
	}

	//判断一张牌是不是癞子牌， 包括王牌
	bool is_joker(const PokerCard& card, int joker_point) {return card.m_point == joker_point || card.m_point >= PokerCard::JOKER1;}
	//判断是不是纯顺子
	bool is_cards_pure_straight(const std::vector<PokerCard>& cards);
	//帕努单
	bool is_cards_straight(const std::vector<PokerCard>& one_group, int joiker_point);
	bool is_cards_three_of_kind(const std::vector<PokerCard>& one_group, int joker_point);
	bool is_groups_declare(const std::vector<CardGroup>& groups, int joker_point);
	bool check_lose_declare(const std::vector<CardGroup>& groups, int joker_point, int& score);
	void default_declare(const std::vector<PokerCard>& hand_card, int joker_point, 
		int& score, std::vector<CardGroup>& card_group);
	void post_best_card(const std::vector<PokerCard>& hand_card, int joker_point, PokerCard& card, int& score);
	
	std::string cards_to_string(const std::vector<PokerCard>& cards);

private:
	void post_less_relation_card(const std::vector<PokerCard>& hand_card, int joker_point, PokerCard& card);
	void caculate_cards_relation(std::vector<CardRelation>& card_relatin);
	void post_valid_group_bigest_card(const std::vector<CardGroup>& card_groups, int joker_point, PokerCard& card);
	void search_joker_cards(const std::vector<PokerCard> &all_cards, std::vector<PokerCard> &joker_cards);
	void search_all_group(const std::vector<PokerCard> &all_cards, size_t valid_card_num_stop, std::vector<CombinationInfo> & card_combs);
	void search_three_of_kind(const std::vector<PokerCard> &all_cards, std::vector<CardGroup> & card_group);
	void search_three_of_kind_i(const std::vector<PokerCard> &all_cards, const std::vector<PokerCard> &reteat_card,
		std::vector<CardGroup> & card_group);
	void search_three_of_kind_select_n(const std::vector<PokerCard> &all_cards, int select_n, std::vector<CardGroup> & card_group);
	void search_straight_flush(const std::vector<PokerCard> &all_cards, std::vector<CardGroup> & card_group);
	void search_straight_flush_i(const std::vector<PokerCard> &flush_cards, const std::vector<PokerCard> &reteat_card, std::vector<CardGroup> & card_group);
	void search_straight_flush_select_n(const std::vector<PokerCard> &all_cards, int select_n, std::vector<CardGroup> & card_group);
	void search_pure_straight_flush(const std::vector<CardGroup> & card_group, std::vector<CardGroup> & pure_group);
	//void remove_cards(const CardGroup& one_group, const std::vector<PokerCard> &all_cards, std::vector<PokerCard> &out_cards);
	void remove_cards(const CardGroup& one_group, const std::vector<PokerCard> &all_cards, std::vector<PokerCard> &out_cards);
	void remove_card_group(const std::vector<PokerCard> &all_cards, const std::vector<CardGroup>& card_group, std::vector<CardGroup>& out_group);
	bool is_card_group_valid(const CardGroup& one_group, const std::list<PokerCard> &all_cards) {}
	void gen_valid_card_group(const std::vector<PokerCard> &all_cards, CardGroup& card_group, int& score);
	bool is_card_in_group(const PokerCard& card, const std::vector<PokerCard> &all_cards);
	void select_card_combine(const std::vector<PokerCard> &all_cards, int select_n,
		std::vector<std::vector<PokerCard> >& all_combines); //从一堆牌当中选几个牌的组合有多少
	void to_straight_fill_joker_num(const std::vector<PokerCard> & cards, int& min_num, int& max_num);
	void combine_with_joker_card(const std::vector<PokerCard> &all_cards, std::vector<CardGroup> & card_group);
	void fill_cards_with_joker_card(const std::vector<PokerCard>& joker_cards, std::vector<PokerCard> &all_cards);
	void caculate_all_cards_combines(const std::vector<PokerCard>& all_cards, int min_select, int max_select, CARD_COMBINE& out_combines);
	
	template <typename T>
	void index_to_elem(const std::vector<T>& vec_elem, const std::vector<int>& vec_index, std::vector<T>& out_vec) {
		for (size_t i = 0; i < vec_index.size(); ++i) {
			if (vec_index[i] == 1)
			{
				out_vec.push_back(vec_elem[i]);
			}
		}
	}

	template <typename T>
	void caculate_all_choices(const std::vector<T>& arr, int n, std::vector<std::vector<T> >& all_combines) 
	{
		//initial first combination like:1,1,0,0,0
		int total_size = arr.size();
		if (total_size < n)
		{
			return;
		}
		std::vector<int> vec_index(arr.size(), 0);
		for (int i = 0; i < n; ++i) {
			vec_index[i] = 1;
		}

		std::vector<T> one_comb;
		index_to_elem(arr, vec_index, one_comb);
		all_combines.push_back(one_comb);

		for (int i = 0; i < total_size - 1; ++i) {
			if (vec_index[i] == 1 && vec_index[i + 1] == 0) {
				//1. first exchange 1 and 0 to 0 1
				std::swap(vec_index[i], vec_index[i + 1]);

				//2.move all 1 before vecInt[i] to left
				sort(vec_index.begin(), vec_index.begin() + i, std::greater<int>());

				//after step 1 and 2, a new combination is exist
				one_comb.clear();
				index_to_elem(arr, vec_index, one_comb);
				all_combines.push_back(one_comb);
				//try do step 1 and 2 from front
				i = -1;
			}
		}

	}

private:
	std::vector<PokerCard> m_joker_cards;
	CARD_COMBINE m_joker_combines;
	int m_joker_point;
};

using CombinationInstance = boost::serialization::singleton<Combination>;
