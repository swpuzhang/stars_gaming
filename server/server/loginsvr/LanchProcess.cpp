#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cstring>
#include <signal.h>

#include "LanchProcess.h"
#include "LoginTaskManager.h"
#include "MqTaskManager.h"
#include "LoginMsgFactor.h"
#include "libserver/Server.h"
#include "libtools/MultiProcess.h"
#include "libtools/IoLoop.h"
#include "libtools/IoLoopPool.h"
#include "libtask_manager/TaskManager.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageMaker.h"
#include "libtools/Types.h"


void LanchProcess::run(int argc, char *argv[])
{
	m_config_file = "./config/config.json";
	m_config_str = FileTool::get_file_content(m_config_file);
	m_argc = argc;
	m_argv = std::vector<std::string>(argv, argv + argc);
	std::shared_ptr<IoLoopPool> io_pool = std::make_shared<IoLoopPool>(1);
#ifndef _WIN32
	std::unique_ptr<boost::asio::signal_set> signals =
		std::make_unique<boost::asio::signal_set>(io_pool->get_next_loop(), SIGPIPE, SIGQUIT, SIGHUP);
	signals->async_wait(std::bind(&LanchProcess::handle_single, this));
#endif // _WIN32

	if (argc == 1)
	{
		m_is_master = true;
	}

	if (m_is_master)
	{
		m_multi_process.creat_process(m_argv[0] + " " + std::to_string(1), 1);
	}
	else
	{
		auto task_io_pool = std::make_shared<IoLoopPool>(1);
		auto mq_io_loop = std::make_shared<IoLoopPool>(1);
		//std::shared_ptr<ThreadPool> th_pool = std::make_shared<ThreadPool>(2);
		auto msg_maker = std::make_shared<LoginMsgFactor>();
		//mq
		auto mq_task = std::make_shared<MqTaskManager>(task_io_pool->get_next_loop());
		auto mq_dealer = std::make_shared<MessageDealer<MqTag>>(mq_task);
		m_mq_client = std::make_unique<MqClient>(io_pool->get_next_loop(),
			m_config_str, mq_dealer, msg_maker);
		m_mq_client->connect();

		//tcpserver
		auto task_manager = std::make_shared<LoginTaskManager>(task_io_pool->get_next_loop());
		auto msg_dealer = std::make_shared<MessageDealer<TcpTag>>(task_manager);
		m_tcpserver = std::make_unique<Server>(io_pool->get_next_loop(),
			msg_dealer, msg_maker);
		m_tcpserver->start(8000);

		bool is_detach = false;
		io_pool->run(is_detach);
		task_io_pool->run(is_detach);
		//th_pool->run(is_detach);
	}
	io_pool->run(false);
}