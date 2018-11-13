#include "LanchProcess.h"
#include "GameMsgFactor.h"
#include "Botman.h"
#include "libserver/Server.h"
#include "libserver/Client.h"
#include "libtools/MultiProcess.h"
#include "libtools/IoLoopPool.h"
#include "libtools/IoLoop.h"
#include "libtask_manager/TaskManager.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageMaker.h"
#include "libmessage/MessageBuffer.h"
#include "libmessage/SendQueue.h"
#include "libtools/Types.h"
#include "libserver/Client.h"
#include "libmessage/Message.h"
#include "libtools/JsonParser.h"
#include "libtools/BaseTool.h"
#include "libtools/JsonParser.h"

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cstring>
#include <thread>
#include <signal.h>


#include "ErrorCode.pb.h"
#include "HallMsg.pb.h"
#include "HallCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace HallCmd;
using namespace HallMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

std::string LanchProcess::get_short_filename(const std::string& str)
{
	std::string short_name = str;
#ifdef _WIN32
	size_t index = short_name.rfind('\\');
#else
	size_t index = short_name.rfind('/');
#endif
	if (index == std::string::npos)
	{
		index = 0;
	}
	else
	{
		++index;
	}
	short_name = short_name.substr(index);
	return "./" + short_name;
}

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

	DEBUG_LOG << "argc:" << argc;
	if (argc == 1)
	{
		m_is_master = true;
	}
	DEBUG_LOG << "is_master:" << m_is_master;
	if (m_is_master)
	{
		DEBUG_LOG << "creat process" << m_argv[0] << " client";
		m_multi_process.creat_process(m_argv[0] + " client", 1);
	}
	else
	{
		std::shared_ptr<IoLoopPool> io_pools = std::make_shared<IoLoopPool>(std::thread::hardware_concurrency());
		std::shared_ptr<IoLoopPool> client_io_pools = std::make_shared<IoLoopPool>(1);
		std::vector<std::shared_ptr<Botman>> botmans;
		for (int i = 0; i < 100; ++i)
		{
			auto one_man = std::make_shared<Botman>(io_pools->get_next_loop(), client_io_pools->get_next_loop(), 10000500 + i);
			botmans.push_back(one_man);
			one_man->open();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		
		bool is_detach = false;
		io_pools->run(is_detach);
		//th_pool->run(is_detach);
	}
	io_pool->run(false);
}