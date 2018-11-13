#include "GG_RandAction.h"
#include <stdlib.h>
#include <assert.h>

const int MAX_ACTION_COUNT = 100;

GG_RandAction_Dispatcher::GG_RandAction_Dispatcher(void)
	: m_new_action_begin(0)
{
	m_actions.resize(MAX_ACTION_COUNT);
}


GG_RandAction_Dispatcher::~GG_RandAction_Dispatcher(void)
{
}

void GG_RandAction_Dispatcher::exec(void)
{
	int action_index = rand() % MAX_ACTION_COUNT;
	m_actions[action_index].exec();
}

int GG_RandAction_Dispatcher::add_action(int chance_percent, ActionHandler action_handler)
{
	assert((m_new_action_begin + chance_percent) <= 100 && chance_percent > 0);
	for (int i = m_new_action_begin; i < (m_new_action_begin + chance_percent); ++i)
	{
		m_actions[i] = action_handler;
	}
	m_new_action_begin = m_new_action_begin + chance_percent;
	return 0;
}

