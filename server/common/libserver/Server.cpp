#include "Server.h"
#include "Session.h"
#include "SessionManager.h"
#include "ServerSessionManager.h"
#include "libmessage/MessageDealer.h"
#include "libtools/IoLoopPool.h"
#include "liblog/Log.h"

#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/function.hpp>
#include <memory>
#include <functional>
#include <thread>
#include <functional>

Server::Server(IoLoop& io_loop, const std::shared_ptr<MessageDealer<TcpTag>>& msg_dealer,
	const std::shared_ptr<MessageMaker>& msg_maker, int thread_num, int max_sessoin) :
		m_acc_loop(io_loop),
		m_accptor(new TCP_SPACE::acceptor(m_acc_loop)),
		m_message_dealer(msg_dealer),
		m_msg_maker(msg_maker)
{
	m_loop_pool = std::make_shared<IoLoopPool>(thread_num);
	m_session_manager = std::make_shared<ServerSessionManager>(m_loop_pool->get_next_loop(), max_sessoin);
}

void Server::start(const int port_num)
{
	TRACE_FUNCATION();
	try
	{

		TCP_SPACE::endpoint ep(TCP_SPACE::v4(), port_num);
		m_accptor->open(ep.protocol());
		m_accptor->set_option(TCP_SPACE::acceptor::reuse_address(true));
		m_accptor->bind(ep);
		m_accptor->listen();
		accept();
	}
	catch (std::exception &e)
	{
		LOG_FATAL(e.what());
	}

	m_loop_pool->run();
}

void Server::accept()
{
	TRACE_FUNCATION();
	std::shared_ptr<TcpSession> new_session = std::make_shared<TcpSession>
		(m_loop_pool->get_next_loop(),
			nullptr, m_message_dealer, m_msg_maker, m_session_manager.get());
	m_accptor->async_accept(new_session->get_socket(), std::bind(&Server::on_accept_complete, this,
		PLACEHOLDER::_1, new_session));
}

void Server::on_accept_complete(const SYSTEM_CODE &err_code, std::shared_ptr<TcpSession> new_session)
{
	TRACE_FUNCATION();
	if (!err_code)
	{
		INFO_LOG << "accept from" << new_session->get_socket().remote_endpoint().address().to_string()
			<< ":" << new_session->get_socket().remote_endpoint().port();
		m_session_manager->add_session(new_session);
	}
	accept();
}
