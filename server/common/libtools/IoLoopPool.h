#pragma once
#include "Types.h"
#include "IoLoop.h"
#include <vector>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <functional>
#include <memory>
#include <thread>
#include <future>
#include <mutex>

class ThreadPool : private boost::noncopyable
{
public:
	explicit ThreadPool(TY_UINT32 thread_size);
	void run(bool is_detach = true);
	void stop();
	void post_handler(std::function<void(void)> func);
	boost::asio::io_context& get_io_context() { return *m_io_context; }
private:
	using io_context_ptr = std::shared_ptr<boost::asio::io_context> ;
	using io_work = boost::asio::executor_work_guard<
		boost::asio::io_context::executor_type>;
	using strand_ptr = std::shared_ptr<boost::asio::io_context::strand> ;
	io_context_ptr m_io_context;
	io_work m_work;
	strand_ptr m_strand;
	std::vector<std::shared_ptr<std::thread> > m_threads;
};
  
class IoLoopPool : private boost::noncopyable
{
public:
	using IoLoopPtr = std::shared_ptr<IoLoop> ;
	/// Construct the io_service pool.
	explicit IoLoopPool(TY_UINT32 pool_size, bool need_mongo = true, bool need_redis = true);

	/// Run all io_service objects in the pool.
	void run(bool is_detach = true);

	/// Stop all io_service objects in the pool.
	void stop();

	/// Get an io_service to use.
	IoLoop& get_loop_with_hash(TY_UINT32 id);

	std::size_t pool_size() { return  m_io_loops.size(); }

	IoLoop& get_next_loop();

	IoLoopPtr& get_next_loop_ptr();

	void init_resource();

	std::vector<std::thread::id> get_all_thread_ids();
private:
	
	using io_work = boost::asio::executor_work_guard<
		boost::asio::io_context::executor_type>;

	/// The pool of io_services.
	std::vector<IoLoopPtr> m_io_loops;

	/// The work that keeps the io_services running.
	std::vector<io_work> m_work;
	 
	TY_UINT32 m_next_index;

	std::vector<std::shared_ptr<std::thread> > m_threads;

	bool m_is_need_mongo;
	bool m_is_need_redis;
};