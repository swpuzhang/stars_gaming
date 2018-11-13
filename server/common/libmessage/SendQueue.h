#ifndef SENDQUEUE_H
#define SENDQUEUE_H
#include "libtools/Types.h"

#include "Message.h"

#include <vector>
#include <boost/circular_buffer.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>


class SendQueue
{
public:
	enum
	{
		INIT_SIZE = 0x100,
		MAX_SIZE = 100000,
	};
	typedef boost::circular_buffer<TcpMsgPtr>::size_type SIZE_TYPE;
	SendQueue();
	bool push(TcpMsgPtr msg);
	TcpMsgPtr pop();
	TcpMsgPtr front();
	bool is_empty() { return m_message_queue.empty();}
private:
	boost::circular_buffer<TcpMsgPtr> m_message_queue;
	
};
#endif