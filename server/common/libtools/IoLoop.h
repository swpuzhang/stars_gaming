#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <functional>
class IoLoop
{
public:
	friend class IoLoopPool;
	boost::asio::io_context& get_io_context() { return m_io_context; }
	bool is_in_loop_thread() {return std::this_thread::get_id() == m_thread_id;}
	void run() 
	{
		m_thread_id = std::this_thread::get_id();
		m_promise.set_value(m_thread_id);
		m_io_context.run(); 
	}
	std::thread::id get_future_thread_id() {
		return  m_promise.get_future().get();
	}
	void stop() { m_io_context.stop(); }
	operator boost::asio::io_context&() { return  m_io_context; }
	void post(std::function<void(void)> f) 
	{ 
		if (is_in_loop_thread())
		{
			f();
		}
		else
		{
			m_io_context.post(f);
		}	
	}
private:
	void set_id(std::thread::id id) { m_thread_id = id; }
	//std::thread::id get_thread_id() { return m_thread_id; }
private:
	boost::asio::io_context m_io_context;
	std::thread::id m_thread_id;
	std::promise<std::thread::id> m_promise;
};