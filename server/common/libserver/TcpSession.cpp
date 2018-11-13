#include <boost/lexical_cast.hpp>
#include <boost/thread/future.hpp>
#include <chrono>
#include <numeric>

#include "MsgType.pb.h"
#include "SystemMsg.pb.h"
#include "SystemCmd.pb.h"

#include "TcpSession.h"
#include "Client.h"

#include "libtools/FunctionBindMacro.h"
#include "libmessage/Message.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageMaker.h"
#include "libmessage/SendQueue.h"
#include "libmessage/MessageBuffer.h"
#include "libtools/IoLoop.h"

#include "liblog/Log.h"

TcpSession::TcpSession(IoLoop& ioloop, SokcetPtr new_socket,
	const std::shared_ptr<MessageDealer<TcpTag>>& msg_dealer,
	const std::shared_ptr<MessageMaker>& msg_maker,
	SessionManager* session_manager): 
					 Session<TcpTag>(ioloop, msg_dealer, msg_maker),
					 m_socket_ptr( new_socket ? new_socket : 
						 SokcetPtr(new TCP_SPACE::socket(ioloop))),
					 m_socket(*m_socket_ptr), 
					 m_send_queue(new SendQueue),
					 m_alive_time(static_cast<int>(::time(NULL))),
					 m_session_id(0),
					 m_alive_interval(20),  
					 m_session_manager(session_manager)
{
	
	//TCP_SPACE::no_delay no_delay(true);
	//m_socket.set_option(no_delay);
}



void TcpSession::do_close()
{
	if (m_socket.is_open())
	{
		FULL_MSG(TcpTag::HeaderType, head, SystemMsg::SessionCloseRequest,
			do_closereq, TcpTag, msg);
		head->set_cmdtype(SystemCmd::CMD_SESSION_CLOSE);
		head->set_type(MsgType::NOTIFY_REQUEST);
		msg->set_session(std::dynamic_pointer_cast<TcpSession>(shared_from_this()));
		m_socket.close();
		if (m_session_manager)
			m_session_manager->close_session(std::dynamic_pointer_cast<TcpSession>(shared_from_this()));
		//回调到上层，让上层释放session
		m_recv_message_dealer->do_message_deal(msg);
		INFO_LOG << "session(" << this << ") do_close";
	}
}

TcpSession::~TcpSession()
{
	
}

void TcpSession::start()
{
	TRACE_FUNCATION();
	flush_alive();
	set_addr();
	m_io_loop.post(MEMFUN_CAST_BIND(do_read_head_len));
	m_io_loop.post(MEMFUN_CAST_BIND(do_session_open));
}

void TcpSession::do_read_head_len()
{
	TRACE_FUNCATION();
	boost::asio::async_read(m_socket, 
		boost::asio::buffer(m_recv_msg_buf.buffer(), HEADER_LENGTH),
		MEMFUN_CAST_BIND1(on_read_head_len_complete, PLACEHOLDER::_1));
}

void TcpSession::do_read_head()
{
	TRACE_FUNCATION();
	m_recv_msg_buf.decode_header_length();
	if (m_recv_msg_buf.header_length() > MSG_BUF_SIZE)
	{
		this->do_close();
		return;
	}
	boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_msg_buf.header_buffer(),
		m_recv_msg_buf.header_length()), 
		MEMFUN_CAST_BIND1(on_read_head_complete, PLACEHOLDER::_1));
}

void TcpSession::on_read_head_len_complete(const SYSTEM_CODE& err_code)
{
	TRACE_FUNCATION();
	if (!err_code)
	{
		do_read_head();
	}
	else
	{
		LOG_INFO("sessin do_close");
		this->do_close();
	}
}

void TcpSession::on_read_head_complete(const SYSTEM_CODE& err_code)
{
	TRACE_FUNCATION();
	if (!err_code)
	{
		do_read_body();
	}
	else
	{
		LOG_INFO("sessin do_close");
		this->do_close();
	}
}

bool TcpSession::decode_header()
{
	HEADER(TcpTag::HeaderType, header);
	m_cur_msg.set_header(header);
	if (!m_recv_msg_buf.decode_header(m_cur_msg.header()))
	{
		return false;
	}
	if (m_recv_msg_buf.buffer_length() > MSG_BUF_SIZE)
	{
		return false;
	}
	if (header->check_result() ^ (header->cmdtype() | header->type() | header->sequenceid()
		| header->length() | header->response_result()))
	{
		LOG_ERROR("header check result failed");
		return false;
	}
	return true;
}

void TcpSession::do_read_body()
{
	TRACE_FUNCATION();
	if (!this->decode_header())
	{
		this->do_close();
		return;
	}
	boost::asio::async_read(m_socket, boost::asio::buffer(m_recv_msg_buf.body_buffer(), 
		m_recv_msg_buf.body_length ()), MEMFUN_CAST_BIND1(on_read_complete, PLACEHOLDER::_1));
}

void TcpSession::on_read_complete(const SYSTEM_CODE& err_code)
{
	TRACE_FUNCATION();
	
	if (!err_code && make_msg())
	{
		flush_alive();
		MessagePtr<TcpTag> msg = std::make_shared<Message<TcpTag>>(m_cur_msg);
		if (m_recv_message_dealer)
		{
			m_recv_message_dealer->do_message_deal(msg);
		}
		do_read_head_len();
	}
	else
	{
		LOG_INFO("sessin do_close error msg:" + err_code.message());
		this->do_close();
	}
}

bool TcpSession::make_msg()
{
	PbMessagePtr pbmsg = m_msg_maker->make_message(
		m_cur_msg.header().type(), m_cur_msg.header().cmdtype());
	if (!pbmsg)
	{
		WARN_LOG << "make msg cmd:" << m_cur_msg.header().cmdtype() << " failed";
		return false;
	}
	if (!m_recv_msg_buf.decode_message(*pbmsg))
	{
		WARN_LOG << "make msg cmd:" << m_cur_msg.header().cmdtype() << " failed";
		return false;
	}
	m_cur_msg.set_pbmessage(pbmsg);
	m_cur_msg.set_session(std::dynamic_pointer_cast<TcpSession>(shared_from_this()));
	DEBUG_LOG << "recv cmd:" << m_cur_msg.header().cmdtype() << " type:" << m_cur_msg.header().type() << " session:" << this;
	return true;
}


bool TcpSession::encode(Message<TcpTag>& msg)
{
	msg.header().set_length(msg.pbmessage().ByteSize());
	msg.header().set_check_result(msg.header().cmdtype() | msg.header().type() 
		| msg.header().sequenceid()| msg.header().length() 
		| msg.header().response_result());
	m_send_msg_buf.encode_header_length(msg.header().ByteSize());
	if (!m_send_msg_buf.encode_header(msg.header()))
	{
		return false;
	}
	if (!m_send_msg_buf.encode_message(msg.pbmessage()))
	{
		return false;
	}
	return true;
}

void TcpSession::do_write(MessagePtr<TcpTag> msg)
{
	TRACE_FUNCATION();
	bool is_in_write = !m_send_queue->is_empty();
	if (!m_send_queue->push(msg))
	{
		LOG_INFO("send buffer eror");
		this->do_close();
		return;
	}
	if (!is_in_write)
	{
		if (!m_socket.is_open())
		{
			return;
		}
		encode(*msg);
		boost::asio::async_write(m_socket, boost::asio::buffer(m_send_msg_buf.buffer(), 
			m_send_msg_buf.buffer_length()), MEMFUN_CAST_BIND1(on_write_complete, PLACEHOLDER::_1));
	}
}


void TcpSession::send_message(int cmd_type, const PbMessagePtr& msg)
{
	TRACE_FUNCATION();
	EMPTY_MSG(TcpTag, sendmsg);
	HEADER(typename TcpTag::HeaderType, head);
	head->set_cmdtype(cmd_type);
	sendmsg->set_header(head);
	sendmsg->set_pbmessage(msg);
	send(sendmsg);
}


void TcpSession::send_message(int cmd_type, const PbMessagePtr& msg, MessagePtr<TcpTag>& response,
	const std::chrono::milliseconds &millisenconds)
{
	TRACE_FUNCATION();
	EMPTY_MSG(TcpTag, sendmsg);
	HEADER(typename TcpTag::HeaderType, head);
	head->set_cmdtype(cmd_type);
	sendmsg->set_header(head);
	sendmsg->set_pbmessage(msg);
	send(sendmsg, response, millisenconds);
}

void TcpSession::send_message(int cmd_type, const PbMessagePtr& msg, const ASYNC_FUN<TcpTag>& fun,
	const std::chrono::milliseconds &millisenconds,
	IoLoop* io_loop)
{
	TRACE_FUNCATION();
	EMPTY_MSG(TcpTag, sendmsg);
	HEADER(typename TcpTag::HeaderType, head);
	head->set_cmdtype(cmd_type);
	sendmsg->set_header(head);
	sendmsg->set_pbmessage(msg);
	send(sendmsg, fun, millisenconds, io_loop);
}


void TcpSession::on_write_complete(const SYSTEM_CODE& err_code)
{
	TRACE_FUNCATION();
	if(!err_code)
	{
		MessagePtr<TcpTag> msg_front = m_send_queue->front();
		DEBUG_LOG  << "send cmd(" << msg_front->header().cmdtype() <<
			") type(" << msg_front->header().type() << ") session(" << this << ")";
		
		m_send_queue->pop();
		
		if (!m_send_queue->is_empty())
		{
			if (!m_socket.is_open())
			{
				return;
			}
			MessagePtr<TcpTag> msg = m_send_queue->front();
			encode(*msg);
			boost::asio::async_write(m_socket, boost::asio::buffer(m_send_msg_buf.buffer(), 
				m_send_msg_buf.buffer_length()), MEMFUN_CAST_BIND1(on_write_complete, PLACEHOLDER::_1));
		}
	}
	else
	{
		LOG_INFO("sessin do_close error msg:" + err_code.message());
		this->do_close();
	}
}



void TcpSession::flush_alive()
{
	m_alive_time = static_cast<int>(::time(NULL));
}

void TcpSession::check_alive()
{
	m_io_loop.post(MEMFUN_CAST_BIND(do_check_alive));
}

void TcpSession::do_check_alive()
{
	if (!is_alive())
	{
		INFO_LOG << "session(" << this << ") is not alive do_close";
		this->do_close();
	}
}

bool TcpSession::is_alive()
{
	bool ret = m_alive_time + m_alive_interval > ::time(NULL);
	return ret;
}

void TcpSession::set_alive_interval(const int seconds)
{
	m_alive_interval = seconds;
}

void TcpSession::set_addr()
{
	m_addr = m_socket.local_endpoint().address().to_string() + ":" + std::to_string(m_socket.local_endpoint().port());
}