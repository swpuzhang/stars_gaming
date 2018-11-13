#include "LanchProcess.h"
#include "GameTaskManager.h"
#include "GameMsgFactor.h"
#include "RoomManager.h"
#include "libserver/Server.h"
#include "libtools/MultiProcess.h"
#include "libtools/IoLoop.h"
#include "libtools/IoLoopPool.h"
#include "libtask_manager/TaskManager.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageMaker.h"
#include "libtools/Types.h"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cstring>
#include <signal.h>

void LanchProcess::run(const std::string& config_file, int argc, char *argv[])
{
	m_config_file = config_file;
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
		std::vector<TY_UINT16> ports = { 8000, 8001 };
		for (auto &e : ports)
		{
			m_multi_process.creat_process(m_argv[0] + " " + std::to_string(e), 1);
		}
		
	}
	else
	{
		std::shared_ptr<IoLoopPool> task_io_pool = std::make_shared<IoLoopPool>(1);
		//std::shared_ptr<ThreadPool> th_pool = std::make_shared<ThreadPool>(2);
		std::shared_ptr<GameTaskManager> task_manager = std::make_shared<GameTaskManager>(task_io_pool->get_next_loop());
		std::shared_ptr<MessageDealer> msg_dealer = std::make_shared<MessageDealer>(task_manager);
		const std::shared_ptr<GameMsgFactor> msg_maker = std::make_shared<GameMsgFactor>();
		std::unique_ptr<Server> server =  std::make_unique<Server>(io_pool->get_next_loop(), msg_dealer, msg_maker);
		RoomManagerInstance::get_mutable_instance().open(atoi(argv[1]), task_io_pool->get_next_loop_ptr().get());
		server->start(atoi(argv[1]));
		bool is_detach = false;
		io_pool->run(is_detach);
		task_io_pool->run(is_detach);
		//th_pool->run(is_detach);
	}
	io_pool->run(false);
}