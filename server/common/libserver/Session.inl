#ifndef SESSION_INL
#define SESSION_INL


#include "Session.h"
#include "Client.h"

#include "MsgType.pb.h"
#include "SystemMsg.pb.h"
#include "SystemCmd.pb.h"

#include "libtools/FunctionBindMacro.h"
#include "libmessage/Message.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageMaker.h"
#include "libmessage/SendQueue.h"
#include "libmessage/MessageBuffer.h"
#include "libtools/IoLoop.h"

#include "liblog/Log.h"

#include <boost/lexical_cast.hpp>
#include <boost/thread/future.hpp>
#include <chrono>
#include <numeric>

template <typename MsgTag>
Session<MsgTag>::Session(IoLoop& ioloop,
	const std::shared_ptr<MessageDealer<MsgTag>>& msg_dealer,
	const std::shared_ptr<MessageMaker>& msg_maker): 
					 m_io_loop(ioloop),
					 m_recv_message_dealer(msg_dealer),
				     m_msg_maker(msg_maker),
					 m_async_request(ioloop),
	                 m_sequenceid(0) 
{
	
	//TCP_SPACE::no_delay no_delay(true);
	//m_socket.set_option(no_delay);
}

template <typename MsgTag>
void Session<MsgTag>::close_fun()
{
	this->do_close();
}

template <typename MsgTag>
Session<MsgTag>::~Session()
{
	
}

template <typename MsgTag>
void Session<MsgTag>::close()
{
	TRACE_FUNCATION();
	m_io_loop.post(MEMFUN_BIND(close_fun));
}

template <typename MsgTag>
void Session<MsgTag>::do_session_open()
{
	FULL_MSG(typename MsgTag::HeaderType, head, SystemMsg::SessionOpenRequest, 
		do_openreq, MsgTag, msg);
	head->set_cmdtype(SystemCmd::CMD_SESSION_OPEN);
	head->set_type(MsgType::NOTIFY_REQUEST);
	msg->set_session(this->shared_from_this());
	//回调到上层，让上层释放session
	m_recv_message_dealer->do_message_deal(msg);
}

template <typename MsgTag>
void Session<MsgTag>::set_msg_sequenceid(const MessagePtr<MsgTag>& msg)
{
	if (msg->header().type() == 0)
	{
		msg->header().set_type(MsgType::NOTIFY_REQUEST);
	}

	msg->header().set_sequenceid(m_sequenceid);
	++m_sequenceid;
}

template <typename MsgTag>
void Session<MsgTag>::send(const MessagePtr<MsgTag>& msg)
{
	TRACE_FUNCATION(); 
	if (send_pre_deal(msg))
	{
		return;
	}
	m_io_loop.post(MEMFUN_BIND1(do_send_message, msg));
}

template <typename MsgTag>
bool Session<MsgTag>::send_pre_deal(const MessagePtr<MsgTag>& msg)
{
	if (msg->header().cmdtype() == SystemCmd::CMD_SHUTDOWN_SESSION)
	{
		close();
		return true;
	}
	return false;
}

template <typename MsgTag>
void Session<MsgTag>::do_send_message(MessagePtr<MsgTag> msg)
{
	TRACE_FUNCATION();
	set_msg_sequenceid(msg);
	this->do_write(msg);
}

template <typename MsgTag>
void Session<MsgTag>::write_fun(MessagePtr<MsgTag> msg)
{
	this->do_write(msg);
}

template <typename MsgTag>
bool Session<MsgTag>::send_response(const MessagePtr<MsgTag>& msg)
{
	TRACE_FUNCATION();
	m_io_loop.post(MEMFUN_BIND1(write_fun, msg));
	return true;
}

template <typename MsgTag>
MessagePtr<MsgTag> Session<MsgTag>::make_response(const int cmd_type)
{
	auto pbmsg = m_msg_maker->make_message(Message<MsgTag>::RESPONSE_MASK, cmd_type);
	EMPTY_MSG(MsgTag, msg);
	HEADER(typename MsgTag::HeaderType, head);
	msg->set_pbmessage(pbmsg);
	msg->set_header(head);
	return msg;
}

template <typename MsgTag>
void Session<MsgTag>::send(const MessagePtr<MsgTag>& msg, MessagePtr<MsgTag>& response,
	const std::chrono::milliseconds &millisenconds)
{
	TRACE_FUNCATION();
	//如果发送线程和接受线程在一个线程， 不允许这样做
	if (send_pre_deal(msg))
	{
		return;
	}

	if (m_io_loop.is_in_loop_thread())
	{
		assert(0);
	}
	else
	{
		do_send_syn_message(msg, response, millisenconds);
	}
}

template <typename MsgTag>
void Session<MsgTag>::do_send_syn_message(MessagePtr<MsgTag> msg, MessagePtr<MsgTag>& response,
	const std::chrono::milliseconds millisenconds)
{
	msg->header().set_type(MsgType::SYN_REQUEST);
	set_msg_sequenceid(msg);
	response = m_sync_request.wait_response(msg, millisenconds, this->shared_from_this());
}

template <typename MsgTag>
void Session<MsgTag>::send(const MessagePtr<MsgTag>& msg, const ASYNC_FUN<MsgTag>& fun, 
	const std::chrono::milliseconds &millisenconds,
	IoLoop* io_loop)
{
	TRACE_FUNCATION();
	if (send_pre_deal(msg))
	{
		return;
	}
	m_io_loop.post(MEMFUN_BIND4(do_send_asyn_message, msg, fun, millisenconds, io_loop));
}

template <typename MsgTag>
void Session<MsgTag>::do_send_asyn_message(MessagePtr<MsgTag> msg, const ASYNC_FUN<MsgTag> fun, 
	const std::chrono::milliseconds millisenconds,
	IoLoop* io_loop)
{
	TRACE_FUNCATION();
	msg->header().set_type(MsgType::ASYN_REQUEST);
	set_msg_sequenceid(msg);
	m_async_request.send_request(io_loop, msg, fun, millisenconds, this->shared_from_this());
	this->do_write(msg);
}

template <typename MsgTag>
void Session<MsgTag>::response_come(const MessagePtr<MsgTag>& msg)
{
	m_sync_request.response_come(msg);
}

template <typename MsgTag>
void Session<MsgTag>::asyn_response_come(const MessagePtr<MsgTag>& msg)
{
	m_async_request.on_response_come(msg);
}
#endif // !SESSION_H