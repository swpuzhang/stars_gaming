#include "GG_GamePool.h"
#include "GG_Player.h"
#include "GG_GameSeat.h"
#include "GG_GameDesk.h"
#include <assert.h>
#include "logger/ilog.h"

GG_GamePool::GG_GamePool(void)
	: m_chips_count(0)
{
}


GG_GamePool::~GG_GamePool(void)
{
}


void GG_GamePool::put_chips(GG_INT64 count, GG_GameSeat *seat /* = NULL */)
{
	TRACE_FUNCATION();
	m_chips_count += count;
	if (seat)
	{
		LOG_TRACE("seat(" + toString(seat->get_seat_number()) +") put chips(" + toString(count) + ")");
		m_bet_seats.insert(seat);
	}
}

void GG_GamePool::remove_seat(GG_GameSeat *seat)
{
	LOG_TRACE("remove seat(" + toString(seat->get_seat_number()) + ")");
	m_bet_seats.erase(seat);
}

GG_INT64 GG_GamePool::get_chips_count(void)
{
	return m_chips_count;
}

int GG_GamePool::seat_count(void)
{
	return m_bet_seats.size();
}

void GG_GamePool::merge(const GG_GamePool &pool)
{
	TRACE_FUNCATION();
	m_chips_count += pool.m_chips_count;
	m_bet_seats.insert(pool.m_bet_seats.begin(), pool.m_bet_seats.end());
}

bool GG_GamePool::is_seat_in_pool(GG_GameSeat *seat)
{
	return m_bet_seats.find(seat) != m_bet_seats.end();
}

void GG_GamePool::deduct_desk_tips(GG_INT64 desk_tips)
{
	assert(m_chips_count > desk_tips);
	m_chips_count -= desk_tips;
}
