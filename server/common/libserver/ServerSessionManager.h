#pragma once
#include <unordered_map>
#include <memory>
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "libtools/IoLoop.h"
#include "SessionManager.h"


constexpr int output_session_info_interval = 30;

class ServerSessionManager : public SessionManager
{
public:
	typedef std::map<int, std::weak_ptr<TcpSession>> ::iterator IT_TYPE;
	using SELF_TYPE = ServerSessionManager;
	ServerSessionManager(IoLoop& io_loop, const int max_session_count = 100000) : 
		m_io_loop(io_loop), 
		m_max_session_count(max_session_count), 
		m_current_session(0),
		m_check_alive_timer(io_loop), 
		m_output_timer(io_loop)
	{
		register_check_timer();
		register_ouput_timer();
	}
	
	void add_session(const std::shared_ptr<TcpSession>& session_ptr);
	bool is_session_over_limit(void) { return m_session_all.size() >= m_max_session_count; }
	void close_session(const std::shared_ptr<TcpSession>& sessoin) override;
private:
	void do_add_session(std::shared_ptr<TcpSession> session_ptr);
	void do_close_session(std::shared_ptr<TcpSession> session);
	void on_check_handle();
	void on_output();

private:
	IoLoop& m_io_loop;
	static int m_max_session_id;
	size_t m_max_session_count;
	void check_session_alive(void);
	void register_check_timer();
	void register_ouput_timer();
	int m_current_session;
	std::unordered_map<int, std::weak_ptr<TcpSession>> m_session_all;
	boost::asio::deadline_timer m_check_alive_timer;
	boost::asio::deadline_timer m_output_timer;
	static int m_check_interval;
};