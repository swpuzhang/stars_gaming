#include "ServerSessionManager.h"
#include "TcpSession.h"
#include "liblog/Log.h"
#include <functional>
#include "libtools/FunctionBindMacro.h"
#include <boost/bind.hpp>

int ServerSessionManager::m_max_session_id = std::numeric_limits<int>::max();
int ServerSessionManager::m_check_interval = 10;

void ServerSessionManager::add_session(const std::shared_ptr<TcpSession>& session_ptr)
{
	m_io_loop.post(MEMFUN_THIS_BIND1(do_add_session, session_ptr));
}

void ServerSessionManager::do_add_session(std::shared_ptr<TcpSession> session_ptr)
{
	if (is_session_over_limit())
	{
		session_ptr->close();
		return;
	}
	m_current_session = m_current_session + 1 % m_max_session_id;
	session_ptr->set_sesssion_id(m_current_session);
	m_session_all.insert({ m_current_session, std::weak_ptr<TcpSession>(session_ptr)});
	session_ptr->start();
}

void ServerSessionManager::check_session_alive(void)
{
	for (auto it = m_session_all.begin(); it != m_session_all.end();)
	{
		std::weak_ptr<TcpSession>& session = it->second;
		std::shared_ptr<TcpSession> session_ptr = session.lock();
		if (!session_ptr)
		{
			m_session_all.erase(it++);
			continue;
		}	
		session_ptr->check_alive();
		++it;
	}
}

void ServerSessionManager::on_check_handle()
{
	check_session_alive();
	register_check_timer();
}

void ServerSessionManager::on_output()
{
	INFO_LOG << "session_count(" << m_session_all.size() << ")";
	register_ouput_timer();
}

void ServerSessionManager::register_check_timer()
{
	m_check_alive_timer.expires_from_now(boost::posix_time::seconds(m_check_interval));
	m_check_alive_timer.async_wait(std::bind(&ServerSessionManager::on_check_handle, this));
}


void ServerSessionManager::close_session(const std::shared_ptr<TcpSession>& sessoin)
{
	m_io_loop.post(MEMFUN_THIS_BIND1(do_close_session, sessoin));
}


void ServerSessionManager::register_ouput_timer()
{
	m_output_timer.expires_from_now(boost::posix_time::seconds(output_session_info_interval));
	m_output_timer.async_wait(std::bind(&ServerSessionManager::on_output, this));

}

void ServerSessionManager::do_close_session(std::shared_ptr<TcpSession> session)
{
	m_session_all.erase(session->get_session_id());
	session->set_sesssion_id(-1);
}