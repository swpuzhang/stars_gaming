#include "Combination.h"
#include "libmessage/Message.h"
#include "liblog/Log.h"
#include <map>
#include <iterator>
#include <assert.h>
#include <array>



static bool sort_combine_func(const CombinationInfo& left_comb, const CombinationInfo& right_comb)
{
	//排序组合选出最优， 首先是有firstlife，然后有secondlife， 然后是散牌最少， 然后去分数最少。 然后从散牌中选出一个无关联的分数最高的牌
	if (left_comb.m_is_has_firtlife == right_comb.m_is_has_firtlife)
	{
		if (left_comb.m_is_has_firtlife)
		{
			if (left_comb.m_is_has_secondelife == right_comb.m_is_has_secondelife)
			{
			}
			else
			{
				return left_comb.m_is_has_secondelife;
			}
			
		}

		//计算散牌最少
		if (left_comb.m_valid_cards.size() != right_comb.m_valid_cards.size())
		{
			return left_comb.m_valid_cards.size() < right_comb.m_valid_cards.size();
		}
		return false;
	}

	return left_comb.m_is_has_firtlife;
}

void Combination::search_joker_cards(const std::vector<PokerCard> &all_cards, std::vector<PokerCard> &joker_cards)
{
	TRACE_FUNCATION();
	for (std::vector<PokerCard>::const_iterator it = all_cards.begin(); it != all_cards.end(); ++it)
	{
		if (is_joker(*it, m_joker_point))
		{
			joker_cards.push_back(*it);
		}
	}
}

void Combination::default_declare(const std::vector<PokerCard>& hand_card, int joker_point, int& score, std::vector<CardGroup>& card_group)
{
	TRACE_FUNCATION();
	m_joker_point = joker_point;
	std::vector<CombinationInfo>  card_combs;
	search_all_group(hand_card, 0, card_combs);
	assert(!card_combs.empty());
	if (card_combs.back().m_score == 0)
	{
		card_group = card_combs.back().m_card_group;
		score = card_combs.back().m_score;
		return;
	}

	score = card_combs.front().m_score;
	std::vector<CombinationInfo>::iterator it_min = card_combs.begin();
	for (std::vector<CombinationInfo>::iterator it = card_combs.begin(); it != card_combs.end(); ++it)
	{
		if (it->m_score < score)
		{
			score = it->m_score;
			it_min = it;
		}
	}
	card_group = it_min->m_card_group;
}

void Combination::post_valid_group_bigest_card(const std::vector<CardGroup>& card_groups, int joker_point, PokerCard& card)
{
	TRACE_FUNCATION();
	int score = -1;
	for (std::vector<CardGroup>::const_iterator it = card_groups.begin(); it != card_groups.end(); ++it)
	{
		if (it->m_cards.size() < 4)
		{
			continue;
		}
		
		std::list<PokerCard> list_cards(it->m_cards.begin(), it->m_cards.end());
		list_cards.sort();

		int nonjoker_count = 0;
		std::list<PokerCard>::const_iterator itnojoker = list_cards.begin();
		std::list<PokerCard>::const_iterator itjoker = list_cards.begin();
		std::list<PokerCard>::const_iterator itpost = list_cards.begin();
		for (std::list<PokerCard>::const_iterator itcard = list_cards.begin(); itcard != list_cards.end(); ++itcard)
		{
			if (is_joker(*itcard, joker_point))
			{
				itjoker = itcard;
			}
			else
			{
				++nonjoker_count;
				itnojoker = itcard;
			}
		}
		
		if (nonjoker_count <= 1)
		{
			itpost = itjoker;
		}

		else
		{
			itpost = itnojoker;
		}

		if (itpost->get_score(joker_point) > score)
		{
			score = itpost->get_score(joker_point);
			card = *itpost;
		}
	}

	//三条变同花顺， server不用处理
}

void Combination::post_best_card(const std::vector<PokerCard>& hand_card, int joker_point, PokerCard& card, int& score)
{
	TRACE_FUNCATION();
	m_joker_point = joker_point;
	std::vector<CombinationInfo>  card_combs;
	search_all_group(hand_card, 1, card_combs);
	assert(!card_combs.empty());

	//排序组合选出最优， 首先是有firstlife，然后有secondlife， 然后是散牌最少， 然后去分数最少。 然后从散牌中选出一个无关联的分数最高的牌
	std::sort(card_combs.begin(), card_combs.end(), sort_combine_func);
	std::vector<CombinationInfo>::iterator itmin = card_combs.begin();
	if (itmin->m_valid_cards.empty())
	{
		//说明这组直接就是0分， 打一张>4张的牌
		post_valid_group_bigest_card(itmin->m_card_group, joker_point, card);
		score = 0;
		return;
	}
	post_less_relation_card(itmin->m_valid_cards, joker_point, card);
	int min_score = itmin->m_score - card.get_score(joker_point);

	//将排名相等的 算出出牌后，分数最小的组合
	for (std::vector<CombinationInfo>::iterator it = card_combs.begin() + 1; it != card_combs.end(); ++it)
	{
		if (itmin->m_is_has_firtlife != it->m_is_has_firtlife || itmin->m_is_has_secondelife != it->m_is_has_secondelife
			|| itmin->m_valid_cards.size() != it->m_valid_cards.size())
		{
			break;
		}
		if (it->m_valid_cards.empty())
		{
			//说明这组直接就是0分， 打一张>4张的牌
			post_valid_group_bigest_card(it->m_card_group, joker_point, card);
			score = 0;
			return;
		}

		PokerCard postcard;
		post_less_relation_card(it->m_valid_cards, joker_point, postcard);
		int score = it->m_score - postcard.get_score(joker_point);
		if (score < min_score)
		{
			itmin = it;
			min_score = score;
			card = postcard;
		}
	}
	score = min_score;
}

void Combination::post_less_relation_card(const std::vector<PokerCard>& hand_card, int joker_point, PokerCard& card)
{
	TRACE_FUNCATION();
	assert(!hand_card.empty());
	std::vector<PokerCard> cards(hand_card.begin(), hand_card.end());
	std::sort(cards.begin(), cards.end());	
	std::vector<CardRelation> first_cards;
	std::vector<CardRelation> repeat_cards;
	for (std::vector<PokerCard>::const_iterator it = cards.begin(); it != cards.end(); ++it)
	{
		CardRelation rela;
		if (is_joker(*it, joker_point))
		{
			continue;
		}
		rela.m_card = *it;
		rela.m_relate_degree = 0;
		std::vector<PokerCard>::const_iterator it_next = it + 1;
		if (it_next == cards.end())
		{
			first_cards.push_back(rela);
			break;
		}
		if (*it_next == *it)
		{
			rela.m_relate_degree = -1;
			repeat_cards.push_back(rela);
		}
		else
		{
			first_cards.push_back(rela);
		}
	}

	//全是癞子牌
	if (first_cards.empty())
	{
		//随便打一张
		card = cards.back();
		return;
	}

	//对第一副牌计算维度
	caculate_cards_relation(first_cards);
	caculate_cards_relation(repeat_cards);
	std::copy(repeat_cards.begin(), repeat_cards.end(), std::back_inserter(first_cards));
	std::vector<CardRelation>::iterator it_min = first_cards.begin();
	for (std::vector<CardRelation>::iterator it = first_cards.begin() + 1; it != first_cards.end(); ++it)
	{
		if (it->m_relate_degree < it_min->m_relate_degree)
		{
			it_min = it;
		}
		else if (it->m_relate_degree == it_min->m_relate_degree
			&& it->m_card.get_score(joker_point) > it_min->m_card.get_score(joker_point))
		{
			it_min = it;
		}
	}
	card = it_min->m_card;
}

void Combination::caculate_cards_relation(std::vector<CardRelation>& card_relatin)
{
	TRACE_FUNCATION();
	for (std::vector<CardRelation>::iterator it = card_relatin.begin(); it != card_relatin.end(); ++it)
	{
		for (std::vector<CardRelation>::iterator it_next = it + 1; it_next != card_relatin.end(); ++it_next)
		{
			if (it_next->m_card.m_point == it->m_card.m_point)
			{
				it->m_relate_degree += 2;
				it_next->m_relate_degree += 2;
			}
			//花色相同，点数相隔2
			else if (abs(it_next->m_card.m_point - it->m_card.m_point) == 2 && it_next->m_card.m_color == it->m_card.m_color)
			{
				it->m_relate_degree += 3;
				it_next->m_relate_degree += 3;
			}
			//花色相同，点数相隔1
			else if (abs(it_next->m_card.m_point - it->m_card.m_point) == 1 && it_next->m_card.m_color == it->m_card.m_color)
			{
				it->m_relate_degree += 4;
				it_next->m_relate_degree += 4;
			}
			else if (abs(it_next->m_card.m_point - it->m_card.m_point) > 2)
			{
				break;
			}
		}

		//如果遍历到了A, 那么要向前遍历直到点数大于3为止
		if (it->m_card.m_point == PokerCard::A)
		{
			for (std::vector<CardRelation>::iterator it_next = card_relatin.begin();
				it_next != card_relatin.end() && it_next->m_card.m_point <= 3; ++it_next)
			{
				if (it_next->m_card.m_color == it->m_card.m_color && it_next->m_card.m_point == PokerCard::P2)
				{
					it->m_relate_degree += 4;
					it_next->m_relate_degree += 4;
				}
				else if (it_next->m_card.m_color == it->m_card.m_color && it_next->m_card.m_point == PokerCard::P3)
				{
					it->m_relate_degree += 3;
					it_next->m_relate_degree += 3;
				}
			}
		}
	}

}

void Combination::search_all_group(const std::vector<PokerCard> &all_cards, size_t valid_card_num_stop, std::vector<CombinationInfo> & card_combs)
{
	//TRACE_FUNCATION();
	LOG_DEBUG("search_all_group begin");
	auto begin = std::chrono::high_resolution_clock::now();
	std::vector<CardGroup> strai_group;
	std::vector<CardGroup> fistlife_group;
	std::vector<PokerCard> all_joker_cards;
	search_joker_cards(all_cards, all_joker_cards);
	CARD_COMBINE all_joker_combine;
	caculate_all_cards_combines(all_joker_cards, 1, all_joker_cards.size(), all_joker_combine);

	m_joker_cards = all_joker_cards;
	m_joker_combines = all_joker_combine;
	search_straight_flush(all_cards, strai_group);
	search_pure_straight_flush(strai_group, fistlife_group);

	
	LOG_DEBUG("serch group combine begin");
	TRACE_FUNCATION();
	if (fistlife_group.empty())
	{
		CombinationInfo oneinfo;
		oneinfo.m_valid_cards = all_cards;
		CardGroup onegroup;
		gen_valid_card_group(all_cards, onegroup, oneinfo.m_score);
		oneinfo.m_card_group.push_back(onegroup);
		card_combs.push_back(oneinfo);
		return;
	}

	for (std::vector<CardGroup>::iterator it = fistlife_group.begin(); it != fistlife_group.end(); ++it)
	{
		std::vector<PokerCard> cards1;
		remove_cards(*it, all_cards, cards1);
		std::vector<PokerCard> joker_cards1;;
		remove_cards(*it, all_joker_cards, joker_cards1);
		if (cards1.empty())
		{
			//这种情况是整个牌形成了同花顺, 默认忽略
			continue;
		}

		std::vector<CardGroup> secondlife_group;
		remove_card_group(cards1, strai_group, secondlife_group);

		//没有第二条命， 那么剩余的牌为散牌
		if (secondlife_group.empty())
		{
			CombinationInfo onecomb;
			onecomb.m_is_has_firtlife = true;
			onecomb.m_valid_cards = std::vector<PokerCard>(cards1.begin(), cards1.end());
			CardGroup onegroup;
			gen_valid_card_group(cards1, onegroup, onecomb.m_score);
			onecomb.m_card_group.push_back(*it);
			onecomb.m_card_group.push_back(onegroup);
			card_combs.push_back(onecomb);
			continue;
		}

		for (std::vector<CardGroup>::iterator itSecond = secondlife_group.begin(); itSecond != secondlife_group.end(); ++itSecond)
		{
			//计算剩余手牌所有同花顺和条子
			std::vector<PokerCard> cards2;
			remove_cards(*itSecond, cards1, cards2);
			std::vector<PokerCard> joker_cards2 = joker_cards1;
			remove_cards(*itSecond, joker_cards1, joker_cards2);
			if (cards2.empty())
			{
				CombinationInfo onecomb;
				onecomb.m_is_has_firtlife = true;
				onecomb.m_is_has_secondelife = true;
				onecomb.m_card_group.push_back(*it);
				onecomb.m_card_group.push_back(*itSecond);
				card_combs.push_back(onecomb);
				if (valid_card_num_stop >= 0)
				{
					return;
				}
				continue;
			}

			std::vector<CardGroup> all_group2;
			if (m_joker_cards != joker_cards2)
			{
				m_joker_cards = joker_cards2;
				m_joker_combines.clear();
				caculate_all_cards_combines(joker_cards2, 1, joker_cards2.size(), m_joker_combines);
			}

			search_straight_flush(cards2, all_group2);
			search_three_of_kind(cards2, all_group2);
			if (all_group2.empty())
			{
				CombinationInfo onecomb;
				onecomb.m_is_has_firtlife = true;
				onecomb.m_is_has_secondelife = true;
				onecomb.m_valid_cards = std::vector<PokerCard>(cards2.begin(), cards2.end());
				CardGroup onegroup;
				gen_valid_card_group(cards2, onegroup, onecomb.m_score);
				onecomb.m_card_group.push_back(*it);
				onecomb.m_card_group.push_back(*itSecond);
				onecomb.m_card_group.push_back(onegroup);
				card_combs.push_back(onecomb);
				if (valid_card_num_stop >= 0 && valid_card_num_stop >= cards2.size())
				{
					return;
				}
				continue;
			}

			for (std::vector<CardGroup>::iterator it2 = all_group2.begin(); it2 != all_group2.end(); ++it2)
			{
				std::vector<CardGroup> card_groups3;
				card_groups3.push_back(*it2);
				std::vector<PokerCard> cards3;
				remove_cards(*it2, cards2, cards3);
				if (cards3.empty())
				{
					CombinationInfo onecomb;
					onecomb.m_is_has_firtlife = true;
					onecomb.m_is_has_secondelife = true;
					onecomb.m_card_group.push_back(*it);
					onecomb.m_card_group.push_back(*itSecond);
					onecomb.m_card_group.push_back(*it2);
					card_combs.push_back(onecomb);
					if (valid_card_num_stop >= 0)
					{
						return;
					}
					continue;
				}

				std::vector<CardGroup> all_group3;
				remove_card_group(cards3, all_group2, all_group3);
				if (all_group3.empty())
				{
					CombinationInfo onecomb;
					onecomb.m_is_has_firtlife = true;
					onecomb.m_is_has_secondelife = true;
					onecomb.m_valid_cards = std::vector<PokerCard>(cards3.begin(), cards3.end());
					CardGroup onegroup;
					gen_valid_card_group(cards3, onegroup, onecomb.m_score);
					onecomb.m_card_group.push_back(*it);
					onecomb.m_card_group.push_back(*itSecond);
					onecomb.m_card_group.push_back(*it2);
					onecomb.m_card_group.push_back(onegroup);
					card_combs.push_back(onecomb);
					if (valid_card_num_stop >= 0 && valid_card_num_stop >= cards3.size())
					{
						return;
					}
					continue;
				}

				for (std::vector<CardGroup>::iterator it3 = all_group3.begin(); it3 != all_group3.end(); ++it3)
				{
					std::vector<PokerCard> cards4;
					remove_cards(*it3, cards3, cards4);
					if (cards4.empty())
					{
						CombinationInfo onecomb;
						onecomb.m_is_has_firtlife = true;
						onecomb.m_is_has_secondelife = true;
						onecomb.m_card_group.push_back(*it);
						onecomb.m_card_group.push_back(*itSecond);
						onecomb.m_card_group.push_back(*it2);
						onecomb.m_card_group.push_back(*it3);
						card_combs.push_back(onecomb);
						if (valid_card_num_stop >= 0)
						{
							return;
						}
						continue;
					}

					//最多有4个牌组，剩余牌不可能超过2
					assert(cards4.size() > 0 && cards4.size() < 3);
					//剩余的牌肯定是散牌
					CombinationInfo onecomb;
					onecomb.m_is_has_firtlife = true;
					onecomb.m_is_has_secondelife = true;
					onecomb.m_valid_cards = std::vector<PokerCard>(cards4.begin(), cards4.end());
					CardGroup onegroup;
					gen_valid_card_group(cards4, onegroup, onecomb.m_score);
					onecomb.m_card_group.push_back(*it);
					onecomb.m_card_group.push_back(*itSecond);
					onecomb.m_card_group.push_back(*it2);
					onecomb.m_card_group.push_back(*it3);
					onecomb.m_card_group.push_back(onegroup);
					card_combs.push_back(onecomb);
					if (valid_card_num_stop >= 0 && valid_card_num_stop >= cards4.size())
					{
						return;
					}
				}
			}
		}
	}

}

bool Combination::is_card_in_group(const PokerCard& card, const std::vector<PokerCard> &all_cards)
{
	std::vector<PokerCard>::const_iterator it =  std::find(all_cards.begin(), all_cards.end(), card);
	return it != all_cards.end();
}

void Combination::search_three_of_kind(const std::vector<PokerCard> &all_cards, std::vector<CardGroup> & card_group)
{
	TRACE_FUNCATION();
	std::list<PokerCard> left_cards(all_cards.begin(), all_cards.end());
	left_cards.sort();

	std::vector<PokerCard> pop_cards;
	std::vector<PokerCard> pop_repeat_cards;
	while (!left_cards.empty())
	{
		if ((pop_cards.empty() || 
			(pop_cards.front().m_point == left_cards.front().m_point)) && !is_joker(left_cards.front(), m_joker_point))
		{
			if (is_card_in_group(left_cards.front(), pop_cards))
			{
				pop_repeat_cards.push_back(left_cards.front());
			}
			else
			{
				pop_cards.push_back(left_cards.front());
			}
			left_cards.pop_front();
			continue;
		}
		//2个开始， 加入癞子牌也能成为条子
		if (pop_cards.size() >= 2)
		{
			search_three_of_kind_i(pop_cards, pop_repeat_cards, card_group);
		}
		pop_cards.clear();
		pop_cards.push_back(left_cards.front());
		left_cards.pop_front();
		pop_repeat_cards.clear();
	}

	if (pop_cards.size() >= 2)
	{
		search_three_of_kind_i(pop_cards, pop_repeat_cards, card_group);
	}
}

void Combination::fill_cards_with_joker_card(const std::vector<PokerCard>& joker_cards, std::vector<PokerCard> &all_cards)
{
	
}

void Combination::combine_with_joker_card(const std::vector<PokerCard> &all_cards, std::vector<CardGroup> & card_group)
{
	int min_fill_num = 0;
	int max_fill_num = 0;

	to_straight_fill_joker_num(all_cards, min_fill_num, max_fill_num);
	if (min_fill_num == 0)
	{
		return;
	}

	for (int i = min_fill_num; i <= max_fill_num; ++i)
	{
		for (CARD_COMBINE::iterator itJoker = m_joker_combines.begin(); itJoker != m_joker_combines.end(); ++itJoker)
		{
			if (itJoker->size() != i)
			{
				continue;
			}
			CardGroup onegroup;
			onegroup.m_cards = all_cards;
			std::copy(itJoker->begin(), itJoker->end(), std::back_inserter(onegroup.m_cards));
			std::sort(onegroup.m_cards.begin(), onegroup.m_cards.end());
			onegroup.m_type = CardGroup::GROUP_STAIGHT;
			card_group.push_back(onegroup);
		}
	}
}

void Combination::search_straight_flush_select_n(const std::vector<PokerCard> &all_cards, int select_n, std::vector<CardGroup> & card_group)
{
	TRACE_FUNCATION();
	assert(select_n >= 1 && select_n <= 14);
	if (all_cards.size() < (size_t)select_n)
	{
		return;
	}
	CARD_COMBINE all_select_combines;
	LOG_DEBUG("select_card_combine begin");
	select_card_combine(all_cards, select_n, all_select_combines);
	LOG_DEBUG("select_card_combine end");

	LOG_DEBUG("all_select_combines begin");
	for (CARD_COMBINE::iterator itHand = all_select_combines.begin(); itHand != all_select_combines.end(); ++itHand)
	{
		if (is_cards_pure_straight(*itHand))
		{
			CardGroup onegroup;
			onegroup.m_cards = *itHand;
			onegroup.m_type = CardGroup::GROUP_STAIGHT;
			card_group.push_back(onegroup);
		}

		//把A当做14计算一遍， 把A当做1计算一遍
		combine_with_joker_card(*itHand, card_group);
		if (itHand->size() > 1 && itHand->back().m_point == PokerCard::A)
		{
			std::vector<PokerCard> hand_cards_A_1;
			hand_cards_A_1.push_back(itHand->back());
			std::copy(itHand->begin(), itHand->end() - 1, std::back_inserter(hand_cards_A_1));
			combine_with_joker_card(hand_cards_A_1, card_group);
		}
		
	}
	LOG_DEBUG("all_select_combines end");
}

void Combination::search_three_of_kind_select_n(const std::vector<PokerCard> &all_cards, int select_n, 
	std::vector<CardGroup> & card_group)
{
	assert(select_n >= 2 && select_n <= 4);
	if (all_cards.size() < (size_t)select_n)
	{
		return;
	}
	CARD_COMBINE all_select_combines;
	select_card_combine(all_cards, select_n, all_select_combines);
	
	for (CARD_COMBINE::iterator itHand = all_select_combines.begin(); itHand != all_select_combines.end(); ++itHand)
	{
		if (itHand->size() > 2)
		{
			CardGroup onegroup;
			onegroup.m_cards = *itHand;
			onegroup.m_type = CardGroup::GROUP_THREE_KIND;
			card_group.push_back(onegroup);
		}
		for (CARD_COMBINE::iterator itJoker = m_joker_combines.begin(); itJoker != m_joker_combines.end(); ++itJoker)
		{
			CardGroup onegroup;
			std::copy(itHand->begin(), itHand->end(), std::back_inserter(onegroup.m_cards));
			std::copy(itJoker->begin(), itJoker->end(), std::back_inserter(onegroup.m_cards));
			onegroup.m_type = CardGroup::GROUP_THREE_KIND;
			card_group.push_back(onegroup);
		}
	}
}

void Combination::search_three_of_kind_i(const std::vector<PokerCard> &all_cards, const std::vector<PokerCard> &reteat_card,
	std::vector<CardGroup> & card_group)
{
	for (int i = 2; i <= 4; ++i)
	{
		search_three_of_kind_select_n(all_cards, i, card_group);
	}
	//第二副牌， 不需要和赖子牌组合， 第一副牌已经把所有组合算出来了
	CARD_COMBINE all_repeat_combines;
	for (int i = 3; i <= 4; ++i)
	{
		all_repeat_combines.clear();
		select_card_combine(reteat_card, i, all_repeat_combines);
		for (CARD_COMBINE::iterator it = all_repeat_combines.begin(); it != all_repeat_combines.end(); ++it)
		{
			CardGroup onegroup;
			onegroup.m_cards = *it;
			onegroup.m_type = CardGroup::GROUP_THREE_KIND;
			card_group.push_back(onegroup);
		}
	}
}

void Combination::select_card_combine(const std::vector<PokerCard> &all_cards, int select_n,
	std::vector<std::vector<PokerCard> >& all_combines)
{
	caculate_all_choices(all_cards, select_n, all_combines);
}


void Combination::gen_valid_card_group(const std::vector<PokerCard> &all_cards, CardGroup& card_group, int& score)
{
	card_group.m_cards = all_cards;
	card_group.m_type = CardGroup::GROUP_INVALID;
	score = caculate_card_score(all_cards, m_joker_point);
}


void Combination::search_straight_flush(const std::vector<PokerCard> &all_cards, std::vector<CardGroup> & card_group)
{
	//先找出不包含赖子的同花， 然后和自己组合， 和赖子组合成同花顺
	//最后在纯赖子组合
	TRACE_FUNCATION();
	std::map<TY_UINT8, std::vector<PokerCard> > flush_cards;
	for (std::vector<PokerCard>::const_iterator iter = all_cards.begin(); iter != all_cards.end(); ++iter)
	{
		flush_cards[iter->m_color].push_back(*iter);
	}

	for (std::map<TY_UINT8, std::vector<PokerCard> >::iterator iter = flush_cards.begin(); iter != flush_cards.end(); ++iter)
	{
		std::vector<PokerCard> left_cards = iter->second;
		std::sort(left_cards.begin(), left_cards.end());

		std::vector<PokerCard> pop_cards;
		std::vector<PokerCard> pop_repeat_cards;
		for (std::vector<PokerCard>::iterator it = left_cards.begin(); it != left_cards.end(); ++it)
		{
			if (!is_joker(*it, m_joker_point))
			{
				if (pop_cards.empty())
				{
					pop_cards.push_back(*it);
				}
				else
				{
					if (pop_cards.back().m_point == it->m_point)
					{
						pop_repeat_cards.push_back(*it);
					}
					else
					{
						pop_cards.push_back(*it);
					}
				}
			}
		}
		search_straight_flush_i(pop_cards, pop_repeat_cards, card_group);
	}

	//全是赖子牌的顺子情况
	for (CARD_COMBINE::iterator itJoker = m_joker_combines.begin(); itJoker != m_joker_combines.end(); ++itJoker)
	{
		if (itJoker->size() < 3)
		{
			continue;
		}
		CardGroup onegroup;
		onegroup.m_cards = *itJoker;
		onegroup.m_type = CardGroup::GROUP_STAIGHT;
		card_group.push_back(onegroup);
	}
}

void Combination::search_straight_flush_i(const std::vector<PokerCard> &flush_cards, const std::vector<PokerCard> &reteat_card, std::vector<CardGroup> & card_group)
{
	TRACE_FUNCATION();
	//算出选择1张非赖子，到N张非赖子 单独组合  以及和赖子组合的所有牌组
	for (size_t i = 1; i <= (flush_cards.size() + m_joker_cards.size()); ++i)
	{
		search_straight_flush_select_n(flush_cards, i, card_group);
	}
	//第二副牌， 不需要和赖子牌组合， 第一副牌已经把所有组合算出来了
	CARD_COMBINE all_repeat_combines;
	for (size_t i = 1; i <= reteat_card.size(); ++i)
	{
		all_repeat_combines.clear();
		select_card_combine(reteat_card, i, all_repeat_combines);
		for (CARD_COMBINE::iterator it = all_repeat_combines.begin(); it != all_repeat_combines.end(); ++it)
		{
			if (is_cards_pure_straight(*it))
			{
				CardGroup onegroup;
				onegroup.m_cards = *it;
				onegroup.m_type = CardGroup::GROUP_STAIGHT;
				card_group.push_back(onegroup);
			}
		}
	}
}

void Combination::to_straight_fill_joker_num(const std::vector<PokerCard> & cards, int& min_num, int& max_num)
{
	assert(!cards.empty());
	int fill_num = 0;
	std::vector<PokerCard>::const_iterator it_last = cards.begin();
	for (std::vector<PokerCard>::const_iterator it = cards.begin() + 1; it != cards.end(); ++it)
	{
		if (it_last->m_point == PokerCard::A)
		{
			fill_num += (it->m_point - 1 - 1);
		}
		else
		{
			fill_num += (it->m_point - it_last->m_point - 1);
		}

		it_last = it;
	}
	
	int joker_card_num = m_joker_cards.size();
	int total_num = cards.size() + fill_num;
	if (total_num < 3)
	{
		fill_num += (3 - total_num);
		total_num = 3;
	}

	//说明已经是个顺子了， 至少要包含一个赖子
	if (fill_num == 0)
	{
		fill_num = 1;
	}

	if (fill_num > joker_card_num)
	{
		min_num = 0;
		max_num = 0;
		return;
	}
	joker_card_num = joker_card_num - fill_num; //剩余joker数量
	min_num = fill_num;
	max_num = fill_num + joker_card_num;
}

void Combination::search_pure_straight_flush(const std::vector<CardGroup> & card_group, std::vector<CardGroup> & pure_group)
{
	TRACE_FUNCATION();
	for (std::vector<CardGroup>::const_iterator it = card_group.begin(); it != card_group.end(); ++it)
	{
		if (is_cards_pure_straight(it->m_cards))
		{
			pure_group.push_back(*it);
		}
	}
}

void Combination::remove_cards(const CardGroup& one_group, const std::vector<PokerCard> &all_cards, std::vector<PokerCard> &out_cards)
{
	out_cards = all_cards;
	for (std::vector<PokerCard>::const_iterator it = one_group.m_cards.begin(); it != one_group.m_cards.end(); ++it)
	{
		std::vector<PokerCard>::iterator itFind = std::find(out_cards.begin(), out_cards.end(), *it);
		if (itFind != out_cards.end())
		{
			out_cards.erase(itFind);
		}
	}
}

void Combination::remove_card_group(const std::vector<PokerCard> &all_cards, const std::vector<CardGroup>& card_group, std::vector<CardGroup>& out_group)
{
	for (std::vector<CardGroup>::const_iterator it = card_group.begin(); it != card_group.end(); ++it)
	{
		bool is_in_cards = true;
		std::array<PokerCard, MAX_CARD_COUNT> arr_card;
		std::copy(all_cards.begin(), all_cards.end(), arr_card.begin());
		//std::list<PokerCard> temp_card = all_cards;
		for (std::vector<PokerCard>::const_iterator itCard = it->m_cards.begin(); itCard != it->m_cards.end(); ++itCard)
		{
			std::array<PokerCard, MAX_CARD_COUNT>::iterator itFind = std::find(arr_card.begin(), arr_card.end(), *itCard);
			if (itFind == arr_card.end())
			{
				is_in_cards = false;
				break;
			}
			itFind->m_point = 0;
			itFind->m_color = 0;
		}
		if (is_in_cards)
		{
			out_group.push_back(*it);
		}
	}
}

void Combination::caculate_all_cards_combines(const std::vector<PokerCard>& all_cards, int min_select, int max_select, CARD_COMBINE& out_combines)
{
	TRACE_FUNCATION();
	for (int i = min_select; i <= max_select; ++i)
	{
		select_card_combine(all_cards, i, out_combines);
	}
}


bool Combination::is_cards_three_of_kind(const std::vector<PokerCard>& one_group, int joker_point)
{
	if (one_group.size() < 3 || one_group.size() > 4)
	{
		DEBUG_LOG << "is_cards_three_of_kind false size:" << (one_group.size());
		return false;
	}
	int un_joker_count = 0;
	
	std::vector<PokerCard>::const_iterator itLast = one_group.begin();
	bool is_last_is_joker = true;


	if (!is_joker(*itLast, joker_point))
	{
		is_last_is_joker = false;
		DEBUG_LOG << "is_last_is_joker set false point:" << (itLast->m_point);
		++un_joker_count;
	}

	for (std::vector<PokerCard>::const_iterator it = one_group.begin() + 1; it != one_group.end(); ++it)
	{
		if (is_joker(*it, joker_point))
		{
			LOG_DEBUG("is_last_is_joker set true");
			is_last_is_joker = true;
			itLast = it;
			continue;
		}
		++un_joker_count;
		
		if (!is_last_is_joker)
		{
			if ((it->m_point != itLast->m_point || it->m_color == itLast->m_color))
			{
				DEBUG_LOG << " is_cards_three_of_kind false point:" << (it->m_point) << "," << (itLast->m_point) <<
					"," << (joker_point);
				return false;
			}
		}

		itLast = it;
		is_last_is_joker = false;
		LOG_DEBUG("is_last_is_joker set fasle");
	}
	if (un_joker_count < 2)
	{
		return false;
	}
	return true;
}

bool Combination::is_cards_straight(const std::vector<PokerCard>& cards, int joker_point)
{
	if (cards.size() < 3)
	{
		return false;
	}
	int joker_count = 0;
	int need_joker_count = 0;
	std::list<PokerCard> tempcards;
	std::vector<PokerCard>::const_iterator itLast = cards.end();
	for (std::vector<PokerCard>::const_iterator it = cards.begin(); it != cards.end(); ++it)
	{
		if (is_joker(*it, joker_point))
		{
			++joker_count;
			continue;
		}
		tempcards.push_back(*it);
		if (itLast == cards.end())
		{
			itLast = it;
			continue;
		}
		if (itLast->m_color != it->m_color)
		{
			return false;
		}

		int diff = it->m_point - itLast->m_point;
		if (diff == 0)
		{
			return false;
		}
		need_joker_count += (diff - 1);
		itLast = it;
	}
	if (need_joker_count <= joker_count)
	{
		return true;
	}
	if (joker_point == PokerCard::A || tempcards.back().m_point != PokerCard::A)
	{
		return false;
	}
	//将A变成1在试一次
	for (std::list<PokerCard>::iterator it = tempcards.begin(); it != tempcards.end(); ++it)
	{
		if (it->m_point == PokerCard::A)
		{
			it->m_point = 1;
		}
	}
	tempcards.sort();
	need_joker_count = 0;

	std::list<PokerCard>::iterator itLast1 = tempcards.end();

	for (std::list<PokerCard>::iterator it = tempcards.begin(); it != tempcards.end(); ++it)
	{
		if (itLast1 == tempcards.end())
		{
			itLast1 = it;
			continue;
		}
		if (itLast1->m_color != it->m_color)
		{
			return false;
		}

		int diff = it->m_point - itLast1->m_point;
		if (diff == 0)
		{
			return false;
		}
		need_joker_count += (diff - 1);
		itLast1 = it;
	}
	if (need_joker_count <= joker_count)
	{
		return true;
	}
	return false;
}

bool Combination::check_lose_declare(const std::vector<CardGroup>& groups, int joker_point, int& score)
{
	int pure_straight_count = 0;
	int straight_count = 0;
	score = 0;
	for (std::vector<CardGroup>::const_iterator it = groups.begin(); it != groups.end(); ++it)
	{
		std::vector<PokerCard> cards = it->m_cards;
		std::sort(cards.begin(), cards.end());

		if (it->m_type == CardGroup::GROUP_STAIGHT)
		{
			if (is_cards_straight(cards, joker_point))
			{
				LOG_DEBUG("check_lose_declare straight++");
				++straight_count;
			}
			else
			{
				LOG_DEBUG("check_lose_declare straight group error:" + cards_to_string(cards));
				return false;
			}
			if (is_cards_pure_straight(cards))
			{
				LOG_DEBUG("is_groups_declare straight++");
				++pure_straight_count;
			}
		}
		
	}

	for (std::vector<CardGroup>::const_iterator it = groups.begin(); it != groups.end(); ++it)
	{
		//如果pure_straight_count < 1 那么所有的牌为散牌
		//pure_straight_count > 1, straight_count
		std::vector<PokerCard> cards = it->m_cards;
		std::sort(cards.begin(), cards.end());

		if (it->m_type == CardGroup::GROUP_INVALID)
		{
			score += caculate_card_score(cards, joker_point);
		}
		else if (it->m_type == CardGroup::GROUP_STAIGHT)
		{
			if (pure_straight_count < 1)
			{
				LOG_DEBUG("check_lose_declare have no first life");
				return false;
			}
		}
		else if (it->m_type == CardGroup::GROUP_THREE_KIND)
		{
			if (pure_straight_count < 1 || straight_count < 2)
			{
				LOG_DEBUG("check_lose_declare have no first life or second life");
				return false;
			}

			if (!is_cards_three_of_kind(cards, joker_point))
			{
				LOG_DEBUG("check_lose_declare kind group error:" + cards_to_string(cards));
				return false;
			}
		}
		else
		{
			return false;
		}

	}

	
	return true;
}

bool Combination::is_groups_declare(const std::vector<CardGroup>& groups, int joker_point)
{
	TRACE_FUNCATION();
	int pure_straight_count = 0;
	int straight_count = 0;
	for (std::vector<CardGroup>::const_iterator it = groups.begin(); it != groups.end(); ++it)
	{
		if (it->m_type == CardGroup::GROUP_INVALID)
		{
			LOG_DEBUG("is_groups_declare has INVALID CARD");
			
			return false;
		}
		std::vector<PokerCard> cards = it->m_cards;
		std::sort(cards.begin(), cards.end());
		if (it->m_type == CardGroup::GROUP_STAIGHT)
		{
			if (is_cards_straight(cards, joker_point))
			{
				LOG_DEBUG("is_groups_declare straight++");
				++straight_count;
			}
			else
			{
				LOG_DEBUG("is_groups_declare straight group error:" + cards_to_string(cards));
				
				return false;
			}
			if (is_cards_pure_straight(cards))
			{
				++pure_straight_count;
				LOG_DEBUG("is_groups_declare straight++");
			}
		}
		if (it->m_type == CardGroup::GROUP_THREE_KIND)
		{
			if (!is_cards_three_of_kind(cards, joker_point))
			{
				LOG_DEBUG("is_groups_declare kind group error:" + cards_to_string(cards));
				return false;
			}
		}
	}

	if (pure_straight_count < 1 || straight_count < 2)
	{
		DEBUG_LOG << "is_groups_declare pure_straight_count:" << (pure_straight_count) << " straight_count:" << (straight_count);
		return false;
	}
	return true;
}

bool Combination::is_cards_pure_straight(const std::vector<PokerCard>& cards)
{
	if (cards.size() < 3)
	{
		return false;
	}
	std::vector<PokerCard>::const_iterator itLast = cards.begin();
	for (std::vector<PokerCard>::const_iterator it = cards.begin() + 1; it != cards.end(); ++it)
	{
		if (itLast->m_color != it->m_color || itLast->m_point >= PokerCard::JOKER1
			|| it->m_point >= PokerCard::JOKER1)
		{
			return false;
		}
		if ((itLast->m_point + 1) != it->m_point)
		{
			if (it->m_point != PokerCard::A)
			{
				return false;
			}
			PokerCard card_first = *cards.begin();
			if (card_first.m_point == PokerCard::P2)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		itLast = it;
	}
	return true;
}


std::string Combination::cards_to_string(const std::vector<PokerCard>& cards)
{
	std::string str;
	for (std::vector<PokerCard>::const_iterator it = cards.begin(); it != cards.end(); ++it)
	{
		str += it->to_string();
		str += ", ";
	}
	return str;
}