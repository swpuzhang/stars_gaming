#ifndef GG_RAND_ACTION_H__
#define GG_RAND_ACTION_H__

#include <vector>
#include <string>

class GG_RandAction_Handler
{
public:
	GG_RandAction_Handler(void){};
	virtual ~GG_RandAction_Handler(void){};
};

typedef void (GG_RandAction_Handler::*ACTION_HANDLER_CALLBACK)(void);

#define ACTION_HANDLER_LOCAL(call_back) GG_RandAction_Dispatcher::ActionHandler((ACTION_HANDLER_CALLBACK)&call_back, this)
#define ACTION_HANDLER_OUTBIND(call_back, bind_obj) GG_RandAction_Dispatcher::ActionHandler((ACTION_HANDLER_CALLBACK)&call_back, bind_obj)

class GG_RandAction_Dispatcher
{
public:
	struct ActionHandler
	{
		ACTION_HANDLER_CALLBACK m_handler_callback;
		GG_RandAction_Handler *m_handler;
		ActionHandler(ACTION_HANDLER_CALLBACK callback = NULL, GG_RandAction_Handler *action_handler = NULL)
			: m_handler_callback(callback), m_handler(action_handler)
		{

		}
		void exec(void)
		{
			if (m_handler_callback && m_handler)
			{
				(m_handler->*m_handler_callback)();
			}
		}
	};

public:
	GG_RandAction_Dispatcher(void);
	~GG_RandAction_Dispatcher(void);

	void exec(void);
	
	int add_action(int chance_percent, ActionHandler action_handler);

private:
	std::vector<GG_RandAction_Dispatcher::ActionHandler> m_actions;
	int m_new_action_begin;
};

#endif //GG_RAND_ACTION_H__
