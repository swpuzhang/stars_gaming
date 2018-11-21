#include "IoLoopPool.h"
#include "libtools/AsioTypes.h"
#include "libresource/Mongodb.h"
#include "libresource/Redis.h"
#include <thread>
#include <functional>
#include <memory>
#include <assert.h>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

ThreadPool::ThreadPool(TY_UINT32 thread_size) :
	m_io_context(new boost::asio::io_context)
	,m_work(boost::asio::make_work_guard(*m_io_context))
	,m_strand(new boost::asio::io_context::strand(*m_io_context))
{
	assert(thread_size > 0);
	for (std::size_t i = 0; i < thread_size; ++i)
	{
		std::shared_ptr<std::thread> thread(new std::thread(
			boost::bind(&boost::asio::io_context::run, m_io_context.get())));
		m_threads.push_back(thread);
	}
}



void ThreadPool::run(bool is_detach)
{
	for (auto &e : m_threads)
	{
		if (is_detach)
		{
			e->detach();
		}
		else
		{
			e->join();
		}
	}
}

void ThreadPool::stop()
{
	m_io_context->stop();
}

void ThreadPool::post_handler(std::function<void(void)> func)
{
	if (m_threads.size() == 1)
	{
		m_io_context->post(func);
	}
	else
	{
		m_strand->post(func);
	}
}

IoLoopPool::IoLoopPool(TY_UINT32 pool_size, bool need_mongo, bool need_redis) : 
	m_next_index(0),
	m_is_need_mongo(need_mongo),
	m_is_need_redis(need_redis)
{
	assert(pool_size > 0);
	for (std::size_t i = 0; i < pool_size; ++i)
	{
		IoLoopPtr ioloop(new IoLoop);
		m_io_loops.push_back(ioloop);
		m_work.push_back(boost::asio::make_work_guard(ioloop->get_io_context()));
		std::shared_ptr<std::thread> thread(new std::thread(
			boost::bind(&IoLoop::run, ioloop)));
		ioloop->set_id(thread->get_id());
		m_threads.push_back(thread);
	}
}

void IoLoopPool::init_resource()
{
	for (auto &e : m_io_loops)
	{
		std::thread::id one_id = e->get_future_thread_id();
		if (m_is_need_mongo)
		{
			MongodbInstance::get_mutable_instance().insert_one(one_id);
		}
		if (m_is_need_redis)
		{
			RedisInstance::get_mutable_instance().insert_one(one_id);
		}
	}
}

void IoLoopPool::run(bool is_detach)
{
	for (auto &e : m_threads)
	{
		if (is_detach)
		{
			e->detach();
		}
		else
		{
			e->join();
		}
	}
}

void IoLoopPool::stop()
{
	// Explicitly stop all io_services.
	for (std::size_t i = 0; i < m_io_loops.size(); ++i)
		m_io_loops[i]->stop();
}

IoLoop& IoLoopPool::get_loop_with_hash(TY_UINT32 id)
{
	// Use a round-robin scheme to choose the next io_service to use.
	std::size_t index = id % m_io_loops.size();
	return *(m_io_loops[index]);
}

IoLoop& IoLoopPool::get_next_loop()
{
	// Use a round-robin scheme to choose the next io_service to use.
	// Use a round-robin scheme to choose the next io_service to use.
	IoLoop& io_loop = *m_io_loops[m_next_index];
	++m_next_index;
	if (m_next_index == m_io_loops.size())
		m_next_index = 0;
	return io_loop;
}

IoLoopPool::IoLoopPtr& IoLoopPool::get_next_loop_ptr()
{
	IoLoopPtr& io_loop = m_io_loops[m_next_index];
	++m_next_index;
	if (m_next_index == m_io_loops.size())
		m_next_index = 0;
	return io_loop;
}


std::vector<std::thread::id> IoLoopPool::get_all_thread_ids()
{
	std::vector<std::thread::id> all_thread_ids;
	for (auto &e : m_io_loops)
	{
		all_thread_ids.push_back(e->get_future_thread_id());
	}
	return all_thread_ids;
}
