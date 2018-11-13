#include <boost/bind.hpp>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include "../libtools/FunctionBindMacro.h"
#include "Client.h"
#include "../libmessage/SendQueue.h"
#include "../libmessage/Message.h"
#include "../libmessage/BasicMessage.h"

CClient::CClient(boost::asio::io_service& sevice, const std::string& ip, const short port_num) : 
	m_socket(sevice), m_reconnect_interval(3),m_reconnet_timer(sevice),
	m_send_queue(boost::make_shared<CSendQueue>()),
	m_endpoint(boost::asio::ip::address::from_string(ip), port_num),
	m_is_connected(false), m_connect_timer(sevice), m_heart_beat_timer(sevice), m_check_alive_timer(sevice)
{
}

int CClient::m_heart_beat_interval = 5;
int CClient::check_link_alive_interval = 1;
int CClient::m_alive_interval = 2 * m_heart_beat_interval;

void CClient::do_read_head()
{
	boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_msg_buf.buffer(), m_recv_msg_buf.HEADER_LENGTH), 
		MEMFUN_BIND1(on_read_head_complete, PLACEHOLDER::error));
}

void CClient::do_read_body()
{
	m_recv_msg_buf.decode_length();
	boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_msg_buf.body_buffer(), 
		m_recv_msg_buf.body_length()), MEMFUN_BIND1(on_read_complete, PLACEHOLDER::error));
}

void CClient::do_write(MESSAGE_PTR msg)
{
	if (!m_is_connected)
	{
		return;
	}
	bool is_in_write = !m_send_queue->is_empty();
	m_send_queue->push(msg);
	if (!is_in_write)
	{
		msg->encode(&m_send_msg_buf);
		boost::asio::async_write(m_socket, boost::asio::buffer(m_send_msg_buf.buffer(), 
			m_send_msg_buf.buffer_length()), MEMFUN_BIND1(on_write_complete, PLACEHOLDER::error));
	}
}

void CClient::on_write_complete(const SYSTEM_CODE& err_code)
{
	if(!err_code)
	{
		m_send_queue->pop();
		if (!m_send_queue->is_empty())
		{
			boost::this_thread::sleep(boost::posix_time::milliseconds(500));
			MESSAGE_PTR msg = m_send_queue->front();
			msg->encode(&m_send_msg_buf);
			boost::asio::async_write(m_socket, boost::asio::buffer(m_send_msg_buf.buffer(), 
				m_send_msg_buf.buffer_length()), MEMFUN_BIND1(on_write_complete, PLACEHOLDER::error));
		}
	}
	else
	{
		if (m_is_connected)
			register_connect_timer();
	}
}

bool CClient::send_message(MESSAGE_PTR msg)
{
	m_socket.get_io_service().post(MEMFUN_BIND1(do_write, msg));
	return true;
}

void CClient::register_connect_timer()
{
	m_socket.close();
	m_is_connected = false;
	m_reconnet_timer.cancel();
	m_reconnet_timer.expires_from_now(boost::posix_time::seconds(m_reconnect_interval));
	m_reconnet_timer.async_wait(MEMFUN_BIND1(on_wait_reconnect, PLACEHOLDER::error));
}

void CClient::register_heart_beat_timer()
{
	m_heart_beat_timer.expires_from_now(boost::posix_time::seconds(m_heart_beat_interval));
	m_heart_beat_timer.async_wait(MEMFUN_BIND1(on_send_heart_beat, PLACEHOLDER::error));
}

void CClient::register_alive_timer()
{
	m_check_alive_timer.expires_from_now(boost::posix_time::seconds(check_link_alive_interval));
	m_check_alive_timer.async_wait(MEMFUN_BIND1(on_check_alive, PLACEHOLDER::error));
}

void CClient::on_connet_complete(const SYSTEM_CODE& err_code)
{
	if (!m_socket.is_open() || err_code)
	{
		register_connect_timer();
	}
	if(!err_code)
	{
		m_is_connected = true;
		do_read_head();
		register_heart_beat_timer();
		register_alive_timer();
	}
}

void CClient::on_read_head_complete(const SYSTEM_CODE& err_code)
{
	
	if (!err_code)
	{
		do_read_body();
	}
	else
	{
		if (m_is_connected)
			register_connect_timer();
	}
}

void CClient::on_read_complete(const SYSTEM_CODE& err_code)
{
	if (!err_code)
	{
		handle_message();
		do_read_head();
	}
	else
	{
		if (m_is_connected)
			register_connect_timer();
	}
}

void CClient::handle_message()
{
	if (m_message_dealer)
		m_message_dealer->do_message_deal(m_recv_msg_buf, SESSION_PTR());
	
}

void CClient::on_wait_reconnect(const SYSTEM_CODE& err)
{
	if (!err)
	{
		connect();
	}
}

void CClient::connect()
{
	m_socket.async_connect(m_endpoint, MEMFUN_BIND1(on_connet_complete, PLACEHOLDER::error));
	m_connect_timer.expires_from_now(boost::posix_time::seconds(5));
	m_connect_timer.async_wait(MEMFUN_BIND1(on_connect_timeout,PLACEHOLDER::error));
}

void CClient::on_connect_timeout(const SYSTEM_CODE& err)
{
	if (!err)
	{
		m_socket.close();
	}
}

CClient::~CClient()
{
}

void CClient::register_messager_dealer(const boost::shared_ptr<CMessageDealer>& msg_dealer)
{
	m_message_dealer = msg_dealer;
}

void CClient::on_send_heart_beat(const SYSTEM_CODE& err)
{
	if (!err)
	{
		if (!m_is_connected)
		{
			return;
		}
		MESSAGE_PTR msg = boost::make_shared<CHeartBeatRequest>(m_alive_interval);
		send_message(msg);
		register_heart_beat_timer();
	}
}

void CClient::on_check_alive(const SYSTEM_CODE& err)
{
	if (!err)
	{
		if (!m_is_connected)
			return;
		if (m_session_ref.expired())
		{
			return;
		}
		SESSION_PTR session = m_session_ref.lock();
		if (session && !session->check_alive())
		{
			session->get_socket().close();
			return;
		}
		register_alive_timer();
	}
}