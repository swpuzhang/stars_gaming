#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include "Client.h"
#include "TcpSession.h"
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

static int session_id = 0;



Client::Client(IoLoop& ioloop, 
	const std::shared_ptr<MessageDealer<TcpTag>>& msg_dealer,
	const std::shared_ptr<MessageMaker>& msg_maker) :

	m_io_loop(ioloop), 
	m_reconnet_timer(ioloop),
	m_connect_timer(ioloop),
	m_heart_beat_timer(ioloop),
	m_check_alive_timer(ioloop),
	m_message_dealer(msg_dealer),
	m_msg_maker(msg_maker),
	m_reconnect_interval(1),
	m_auto_reconnect(false), 
	m_is_connecting(false), 
	m_is_wait_connect(false),
	m_is_connected(false)
{
	
}

int Client::m_heart_beat_interval = 5;
int Client::check_link_alive_interval = m_heart_beat_interval * 2;
int Client::m_alive_interval = 2 * m_heart_beat_interval;

void Client::register_connect_timer()
{
	TRACE_FUNCATION();
	m_is_wait_connect = true;
	m_reconnet_timer.expires_from_now(boost::posix_time::seconds(m_reconnect_interval));
	m_reconnet_timer.async_wait(MEMFUN_THIS_BIND1(on_wait_reconnect, PLACEHOLDER::_1));
	DEBUG_LOG << "client(" << this << ") register_connect_timer interval("
		<< m_reconnect_interval << ")";
}

void Client::register_heart_beat_timer()
{
	TRACE_FUNCATION();
	m_heart_beat_timer.expires_from_now(boost::posix_time::seconds(m_heart_beat_interval));
	m_heart_beat_timer.async_wait(MEMFUN_THIS_BIND1(on_send_heart_beat, PLACEHOLDER::_1));
	DEBUG_LOG << "client(" << this << ") register_heart_beat_timer interval("
		<< m_heart_beat_interval << ")";
}

void Client::register_alive_timer()
{
	TRACE_FUNCATION();
	m_check_alive_timer.expires_from_now(boost::posix_time::seconds(check_link_alive_interval));
	m_check_alive_timer.async_wait(MEMFUN_THIS_BIND1(on_check_alive, PLACEHOLDER::_1));
	DEBUG_LOG << "client(" << this << ") register_alive_timer interval("
		<< check_link_alive_interval << ")";
}

void Client::on_connet_complete(const SYSTEM_CODE& err_code)
{
	TRACE_FUNCATION();
	m_is_connecting = false;
	m_connect_timer.cancel();
	if (!m_socket->is_open() || err_code)
	{
		//超时重连
		if (m_socket->is_open())
		{
			DEBUG_LOG << "client(" << this << ") local addr(" << m_socket->local_endpoint().address().to_string()
				<< ":" << m_socket->local_endpoint().port() << ") async_connect(" << m_endpoint.address()
				<< ":" << m_endpoint.port() << ") failed";
		}

		FULL_MSG(typename TcpTag::HeaderType, head, 
			SystemMsg::ConnectFailedRequest, failereq, TcpTag, msg);
		head->set_cmdtype(SystemCmd::CMD_CONNECT_FAILED);
		head->set_type(MsgType::NOTIFY_REQUEST);
		m_message_dealer->do_message_deal(msg);
		m_socket = nullptr;
		if (m_auto_reconnect)
		{
			register_connect_timer();
		}
		return;
	}

	m_is_connected = true;
	std::shared_ptr<TcpSession> new_session(new TcpSession(m_io_loop, 
		m_socket, m_message_dealer
		, m_msg_maker, this));
	m_session_ref = new_session;
	m_socket = nullptr;
	new_session->set_alive_interval(m_alive_interval);
	new_session->set_sesssion_id(++session_id);
	new_session->start();
	register_heart_beat_timer();
	register_alive_timer();
}



void Client::on_send_heart_beat(const SYSTEM_CODE& err)
{
	TRACE_FUNCATION();
	if (!err)
	{
		if (!m_is_connected)
		{
			return;
		}
		PB_MSG(HeartbeatRequest, pbmsg);
		HEADER(typename TcpTag::HeaderType, hd);
		EMPTY_MSG(TcpTag, msg);
		hd->set_cmdtype(CMD_HEARTBEAT);
		hd->set_type(MsgType::HEARTBEAT_REQUEST);
		msg->set_header(hd);
		msg->set_pbmessage(pbmsg);
		send_message(msg);
		register_heart_beat_timer();
	}
}


void Client::on_wait_reconnect(const SYSTEM_CODE& err)
{
	m_is_wait_connect = false;
	TRACE_FUNCATION();
	if (!err)
	{
		DEBUG_LOG << "client(" << this <<  ") async_connect(" << m_endpoint.address()
			<< ":" << m_endpoint.port() << ") reconnect timerout";
		connect();
	}	
	else
	{
		DEBUG_LOG << "client(" << this << ") async_connect(" << m_endpoint.address()
			<< ":" << m_endpoint.port() << ") reconnect timer cancle";
	}
}

void Client::on_check_alive(const SYSTEM_CODE& err)
{
	TRACE_FUNCATION();
	if (!err)
	{
		if (!m_is_connected)
			return;
		std::shared_ptr<TcpSession> session = m_session_ref.lock();
		if (session)
		{
			session->check_alive();
			register_alive_timer();
		}
		
	}
}

void Client::connect(const std::string& ip, const short port_num)
{
	TRACE_FUNCATION();
	m_io_loop.post(MEMFUN_THIS_BIND2(do_connect, ip, port_num));
}


void Client::do_connect(const std::string ip, const short port_num)
{
	TRACE_FUNCATION();
	m_auto_reconnect = true;
	m_endpoint = TCP_SPACE::endpoint(boost::asio::ip::address::from_string(ip), port_num);
	if (m_is_connected)
	{
		m_session_ref.lock()->close();
	}
	else if (m_is_connecting)
	{
		m_socket->close();
	}
	else
	{
		if (m_is_wait_connect)
		{
			m_reconnet_timer.cancel();
			m_is_wait_connect = false;
		}
		connect();
	}
}

void Client::connect()
{      
	TRACE_FUNCATION();
	m_socket.reset(new TCP_SPACE::socket(m_io_loop));
	//std::shared_ptr<TcpSession> new_session(new Session(m_io_sevice, nullptr, m_message_dealer
	//	, m_msg_maker), MEMFUN_THIS_BIND1(on_socket_close, PLACEHOLDER::_1));
	m_connect_timer.expires_from_now(boost::posix_time::seconds(5));
	//m_connect_timer.async_wait(MEMFUN_THIS_BIND2(on_connect_timeout, PLACEHOLDER::_1, new_session));
	m_connect_timer.async_wait(MEMFUN_THIS_BIND1(on_connect_timeout, PLACEHOLDER::_1));
	m_socket->async_connect(m_endpoint, MEMFUN_THIS_BIND1(on_connet_complete, PLACEHOLDER::_1));
	m_is_connecting = true;
	DEBUG_LOG << "client(" << this << ") local addr(" << m_socket->local_endpoint().address().to_string()
		<< ":" << m_socket->local_endpoint().port() << ") async_connect(" << m_endpoint.address()
		<< ":" << m_endpoint.port() << ")";
}

void Client::close_client()
{
	TRACE_FUNCATION();
	//关闭套接字， 不重连
	m_io_loop.post(MEMFUN_THIS_BIND(do_close_client));
	
}

void Client::do_close_client()
{
	TRACE_FUNCATION();
	m_auto_reconnect = false;
	if (m_is_connected)
	{
		m_session_ref.lock()->close();
	}
	else if (m_is_connecting)
	{
		m_socket->close();
	}
	else if (m_is_wait_connect)
	{
		m_is_wait_connect = false;
		m_reconnet_timer.cancel();
	}
}

void Client::close_session(const std::shared_ptr<TcpSession>& sessoin)
{
	TRACE_FUNCATION();
	INFO_LOG << "client(" << this << ") close";
	m_is_connected = false;
	m_heart_beat_timer.cancel();
	m_check_alive_timer.cancel();
	connect();
}

/*void Client::on_socket_close(Session* sock)
{
	sock->close();
	delete sock;
	sock = nullptr;
	m_is_connected = false;
	m_heart_beat_timer.cancel();
	m_check_alive_timer.cancel();
	connect();
}*/

std::shared_ptr<TcpSession> Client::session_ptr()
{
	TRACE_FUNCATION();
	if (m_session_ref.expired())
	{
		return std::shared_ptr<TcpSession>();
	}
	std::shared_ptr<TcpSession> session = m_session_ref.lock();
	return session;
}

bool Client::send_message(const std::shared_ptr<Message<TcpTag>>& msg)
{
	TRACE_FUNCATION();
	std::shared_ptr<TcpSession> session = session_ptr();
	if (session)
	{
		session->send(msg);
		return true;
	}
	return false;
}

void Client::on_connect_timeout(const SYSTEM_CODE& err)
{
	TRACE_FUNCATION();
	if (!err)
	{
		if (m_socket && m_socket->is_open())
		{
			DEBUG_LOG << "client(" << this << ") local addr(" << m_socket->local_endpoint().address().to_string()
				<< ":" << m_socket->local_endpoint().port() << ") async_connect(" << m_endpoint.address()
				<< ":" << m_endpoint.port() << ") connet timeout";
			m_socket->close();
		}
	}
	else
	{
		if (m_socket && m_socket->is_open())
		{
			DEBUG_LOG << "client(" << this << ") local addr(" << m_socket->local_endpoint().address().to_string()
				<< ":" << m_socket->local_endpoint().port() << ") async_connect(" << m_endpoint.address()
				<< ":" << m_endpoint.port() << ") timer cancel";
		}
		
	}
}

bool Client::send_message(const std::shared_ptr<Message<TcpTag>>& msg, std::shared_ptr<Message<TcpTag>>& response, 
				  const std::chrono::milliseconds &millisenconds)
{
	TRACE_FUNCATION();
	if (response)
		response.reset();
	std::shared_ptr<TcpSession> session = session_ptr();
	if (session)
	{
		session->send(msg, response, millisenconds);
		return true;
	}
	return false;
}

bool Client::send_message(const std::shared_ptr<Message<TcpTag>>& msg, const ASYNC_FUN<TcpTag>& fun,
	const std::chrono::milliseconds &millisenconds, IoLoop* io_loop)
{
	TRACE_FUNCATION();
	std::shared_ptr<TcpSession> session = session_ptr();
	if (session)
	{
		session->send(msg, fun, millisenconds, io_loop);
		return true;
	}
	return false;
}