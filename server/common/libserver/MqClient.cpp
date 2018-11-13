#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include "MqClient.h"
#include "MqSession.h"
#include "libtools/FunctionBindMacro.h"
#include "libmessage/SendQueue.h"
#include "libmessage/Message.h"
#include "libmessage/MessageMaker.h"
#include "libtools/IoLoop.h"
#include "libtools/IoLoopPool.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageMaker.h"
#include "libtask_manager/TaskManager.h"
#include "liblog/Log.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "MsgHeader.pb.h"
#include "MsgType.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;


MqClient::MqClient(IoLoop& ioloop, const std::string &mq_config_str,
	const std::shared_ptr<MessageDealer<MqTag>>& msg_dealer,
	const std::shared_ptr<MessageMaker>& msg_maker) :
	m_io_loop(ioloop), 
	m_timer(ioloop), 
	m_message_dealer(msg_dealer), 
	m_msg_maker(msg_maker),
	m_interval(1),
	m_auto_reconnect(false),
	m_is_connecting(false)
{
	m_mq_info.parse_config(mq_config_str);
	m_session = std::make_shared<MqSession>(m_io_loop, m_mq_info,
		m_message_dealer, m_msg_maker);
}

void MqClient::register_timer()
{
	TRACE_FUNCATION();
	m_timer.expires_from_now(boost::posix_time::seconds(m_interval));
	m_timer.async_wait(MEMFUN_THIS_BIND1(on_timeout, PLACEHOLDER::_1));
}

void MqClient::connect()
{
	TRACE_FUNCATION();
	m_io_loop.post(MEMFUN_THIS_BIND(do_connect));
}

MqClient::~MqClient()
{
	TRACE_FUNCATION();
	m_timer.cancel();
}

void MqClient::do_connect()
{
	TRACE_FUNCATION();
	if (m_session->is_connected())
	{
		return;
	}
	m_auto_reconnect = true;
	connect_mq();
	register_timer();
}

void MqClient::connect_mq()
{      
	TRACE_FUNCATION();
	m_is_connecting = true;
	m_session->connect();
	DEBUG_LOG << "mq_client(" << this << ") connecting...";
}

void MqClient::close_client()
{
	TRACE_FUNCATION();
	//关闭套接字， 不重连
	m_io_loop.post(MEMFUN_THIS_BIND(do_close_client));
	
}

void MqClient::close_session()
{
	TRACE_FUNCATION();
	if (m_session->is_connected())
	{
		m_session->close();
	}
}

void MqClient::do_close_client()
{
	TRACE_FUNCATION();
	m_auto_reconnect = false;
	m_is_connecting = false;
	close_session();
	m_timer.cancel();
}


bool MqClient::send_message(const std::shared_ptr<Message<MqTag>>& msg)
{
	TRACE_FUNCATION();
	if (m_session)
	{
		m_session->send(msg);
		return true;
	}
	return false;
}

void MqClient::on_timeout(const SYSTEM_CODE& err)
{
	TRACE_FUNCATION();
	if (err)
	{
		return;
	}
	//如果session已经断开连接，而且没有正在连接， 那么需要关闭这个session
	if (m_is_connecting)
	{
		if (m_session->is_connected())
		{
			m_is_connecting = false;
		}
	}
	else
	{
		if (!m_session->is_connected())
		{
			close_session();
		}
		if (m_auto_reconnect && !m_session->is_connected())
		{
			connect_mq();
		}
	}
	register_timer();
}

bool MqClient::send_message(const std::shared_ptr<Message<MqTag>>& msg, 
	std::shared_ptr<Message<MqTag>>& response, 
	const std::chrono::milliseconds &millisenconds)
{
	TRACE_FUNCATION();
	if (response)
		response.reset();
	if (m_session)
	{
		m_session->send(msg, response, millisenconds);
		return true;
	}
	return false;
}

bool MqClient::send_message(const std::shared_ptr<Message<MqTag>>& msg, 
	const ASYNC_FUN<MqTag>& fun,
	const std::chrono::milliseconds &millisenconds, IoLoop* io_loop)
{
	TRACE_FUNCATION();
	if (m_session)
	{
		m_session->send(msg, fun, millisenconds, io_loop);
		return true;
	}
	return false;
}