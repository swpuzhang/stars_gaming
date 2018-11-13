#ifndef MESSAGE_DEALER_INL
#define MESSAGE_DEALER_INL
#include "MessageDealer.h"
#include "MessageMaker.h"
#include "Message.h"
#include "MessageBuffer.h"
#include "libmessage/Message.h"
#include "SendQueue.h"
#include "libtask_manager/TaskManager.h"
#include "libserver/Session.h"
#include "libtools/BigEndLittleEnd.h"
#include "liblog/Log.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "MsgHeader.pb.h"
#include "MsgType.pb.h"

template <typename MsgTag>
MessageDealer<MsgTag>::MessageDealer(const std::shared_ptr<TaskManager<MsgTag>>& task_manager)
	:  m_task_manager(task_manager)
{

}

template <typename MsgTag>
void MessageDealer<MsgTag>::do_message_deal(const MessagePtr<MsgTag>& msg)
{
	TRACE_FUNCATION();
	basic_deal(msg);
}

template <typename MsgTag>
void MessageDealer<MsgTag>::basic_deal(const MessagePtr<MsgTag>& msg)
{
	DEBUG_LOG << "basic_deal cmdtype:" << msg->header().cmdtype() << " type:" << msg->header().type();
	switch (msg->header().type())
	{
	case MsgType::SYN_REQUEST:
	{
		deal_message(msg);
		break;
	}
	case MsgType::NOTIFY_REQUEST:
	{
		deal_message(msg);
		break;
	}
	case MsgType::ASYN_REQUEST:
	{
		deal_message(msg);
		break;
	}
	case MsgType::ASYN_RESPONSE:
	{
		this->deal_asyn_response(msg);
		break;
	}
	case MsgType::SYN_RESPONSE:
	{
		this->deal_syn_response(msg);
		break;
	}
	case MsgType::NOTIFY_RESPONSE:
	{
		//this->dispatch_notify_response(msg);
		break;
	}
	case MsgType::HEARTBEAT_REQUEST:
	{
		deal_heart_beat_message(msg);
		deal_message(msg);
		break;
	}
	case MsgType::HEARTBEAT_RESPONSE:
	{
		deal_heart_beat_response(msg);
		break;
	}
	default:
	{
		WARN_LOG << "unknown msg type(" << (msg->header().type()) + ")";
		break;
	}

	}
}

template <typename MsgTag>
void MessageDealer<MsgTag>::deal_message(const MessagePtr<MsgTag>& msg)
{
	TRACE_FUNCATION();
	assert(m_task_manager);
	m_task_manager->handle_message(msg);
}

template <typename MsgTag>
void MessageDealer<MsgTag>::deal_heart_beat_message(const MessagePtr<MsgTag>& msg)
{
	TRACE_FUNCATION();
	FULL_MSG(typename MsgTag::HeaderType, head, SystemMsg::HeartbeatResponse, heart_res, MsgTag, res);
	heart_res->set_interval(msg->session_ptr()->get_heartbeat_interval());
	res->set_pbmessage(heart_res);
	res->header().set_cmdtype(SystemCmd::CMD_HEARTBEAT);
	res->header().set_type(MsgType::HEARTBEAT_RESPONSE);
	msg->send_response(res);
}

template <typename MsgTag>
void MessageDealer<MsgTag>::deal_heart_beat_response(const MessagePtr<MsgTag>& msg)
{
	//const SystemMsg::HeartbeatResponse& pbres = msg->message<SystemMsg::HeartbeatResponse>();
	//msg->session_ptr()->set_alive_interval(pbres.interval());
}

template <typename MsgTag>
void MessageDealer<MsgTag>::deal_syn_response(const MessagePtr<MsgTag>& msg)
{
	msg->session_ptr()->response_come(msg);
}

template <typename MsgTag>
void MessageDealer<MsgTag>::deal_asyn_response(const MessagePtr<MsgTag>& msg)
{
	msg->session_ptr()->asyn_response_come(msg);
}

#endif // !MESSAGE_DEALER_INL