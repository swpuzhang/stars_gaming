#ifndef GG_GAME_POOL_H__
#define GG_GAME_POOL_H__

#include <set>
#include <list>
#include "GG_Game_Message.h"

class GG_GameSeat;
class GG_GameDesk;

class GG_GamePool
{
public:
	GG_GamePool(void);
	~GG_GamePool(void);

	void put_chips(GG_INT64 count, GG_GameSeat *seat = NULL);

	void remove_seat(GG_GameSeat *seat);

	GG_INT64 get_chips_count(void);

	int seat_count(void);

	void merge(const GG_GamePool &pool);

	void deduct_desk_tips(GG_INT64 desk_tips);

private:
	void compare_cards(void);

	bool is_seat_in_pool(GG_GameSeat *seat);
	
private:
	GG_INT64 m_chips_count;	// 奖池总数
	std::set<GG_GameSeat*> m_bet_seats;  // 当前投注此奖池的在局用户位置
};

#endif  //GG_GAME_POOL_H__

