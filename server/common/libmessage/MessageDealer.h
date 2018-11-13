#ifndef BASIC_MESSAGE_DEALER_H
#define BASIC_MESSAGE_DEALER_H
#include "libtools/Types.h"
#include "MessageType.h"
#include <boost/shared_ptr.hpp>

template <typename MsgTag>
class MessageBuffer;

template <typename MsgTag>
class TaskManager;

template <typename MsgTag>
class Session;

template <typename MsgTag>
class MessageDealer
{
public:
	MessageDealer(const std::shared_ptr<TaskManager<MsgTag>>& task_manager);
	using SELF_TYPE = MessageDealer<MsgTag>;
	void do_message_deal(const MessagePtr<MsgTag>& msg);

private:
	void deal_message(const MessagePtr<MsgTag>& msg);
	void basic_deal(const MessagePtr<MsgTag>& msg);
	void deal_heart_beat_message(const MessagePtr<MsgTag>& msg);
	void deal_heart_beat_response(const MessagePtr<MsgTag>& msg);
	
	void deal_syn_response(const MessagePtr<MsgTag>& msg);
	void deal_asyn_response(const MessagePtr<MsgTag>& msg);
	std::shared_ptr<TaskManager<MsgTag>> m_task_manager;
};

using TcpMsgDealer = MessageDealer<TcpTag>;
using MqMsgDealer = MessageDealer<MqTag>;

#include "MessageDealer.inl"

#endif