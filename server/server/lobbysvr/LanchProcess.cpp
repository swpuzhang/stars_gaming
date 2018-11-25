#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cstring>
#include <signal.h>
#include <cstdlib>
#include <cstring>
#include <boost/algorithm/string.hpp>

#include "LanchProcess.h"
#include "PlayerTaskManager.h"
#include "MqTaskManager.h"
#include "LobbyModule.h"

#include "LobbyMsgFactor.h"
#include "libserver/Server.h"
#include "libtools/MultiProcess.h"
#include "libtools/IoLoop.h"
#include "libtools/IoLoopPool.h"
#include "libtask_manager/TaskManager.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageMaker.h"
#include "libtools/Types.h"
#include "libtools/SystemTool.h"
#include "libresource/Mongodb.h"
#include "libresource/Redis.h"

bool LanchProcess::parse_config()
{
	//获取本server的配置
	std::string process_name = SystemTool::get_process_name();
	Json &jv_server_config = m_config_json[process_name];
	if (jv_server_config.is_valid())
	{
		return false;
	}
	//子进程数量
	int sub_count = atoi(jv_server_config["sub_process"].get<std::string>().c_str());
	//起始端口
	int port = atoi(jv_server_config["port"].get<std::string>().c_str());
	assert(sub_count > 0 && sub_count < 100);
	for (int i = 0; i < sub_count; ++i)
	{
		m_ports.push_back(port + i);
	}

	//获取host配置
	//本机外网IP
	m_local_ip = m_config_json["host_config"]["ip"];
	return true;
}

void LanchProcess::run(int argc, char *argv[])
{
	std::srand(std::time(nullptr));
	m_config_file = "./config/config.json";
	std::string  config_str = FileTool::get_file_content(m_config_file);
	//解析配置文件
	m_config_json = Json::parse(config_str);
	parse_config();
	m_argv = std::vector<std::string>(argv, argv + argc);

	//创建一个主loop 单线程
	std::shared_ptr<IoLoopPool> main_pool = std::make_shared<IoLoopPool>(1);
	
	//处理信号, linux上处理信号, 避免窗口关闭后, 进程退出
#ifndef _WIN32
	std::unique_ptr<boost::asio::signal_set> signals =
		std::make_unique<boost::asio::signal_set>(main_pool->get_next_loop(), SIGPIPE, SIGQUIT, SIGHUP);
	signals->async_wait(std::bind(&LanchProcess::handle_single, this));
#endif // _WIN32

	//解析mongo和redis配置文件
	MongodbInstance::get_mutable_instance().parse_config(m_config_json["mongo"]);
	RedisInstance::get_mutable_instance().parse_config(m_config_json["redis"]);

	if (argc == 1)
	{
		m_is_master = true;
	}

	if (m_is_master)
	{
		int i = 1;
		for (auto & e : m_ports)
		{
			m_multi_process.creat_process(m_argv[0] + " " + std::to_string(i++) + " " + std::to_string(e), 1);
		}
		
		main_pool->run(false);
	}
	else
	{
		int child_id = atoi(m_argv[2].c_str());
		int port =  atoi(m_argv[2].c_str());
		auto task_io_pool = std::make_shared<IoLoopPool>(1);
		auto mq_io_loop = std::make_shared<IoLoopPool>(1);
		//登录模块
		LobbyInstance::get_mutable_instance().open(task_io_pool->get_next_loop());

		//主逻辑线程需要用到mongo和redis
		task_io_pool->init_resource();

		//消息生成器
		auto msg_maker = std::make_shared<LoginMsgFactor>();
		//mq
		auto mq_task = std::make_shared<MqTaskManager>(task_io_pool->get_next_loop());
		auto mq_dealer = std::make_shared<MessageDealer<MqTag>>(mq_task);
		auto local_ip = boost::replace_all_copy(m_local_ip, ".", "_");
		m_mq_client = std::make_unique<MqClient>(main_pool->get_next_loop(),
			config_str, std::to_string(child_id) + "-" + local_ip + "-" + std::to_string(port),  mq_dealer, msg_maker);
		m_mq_client->connect();

		//tcpserver
		auto task_manager = std::make_shared<PlayerTaskManager>(task_io_pool->get_next_loop());
		auto msg_dealer = std::make_shared<MessageDealer<TcpTag>>(task_manager);
		m_tcp_server = std::make_unique<Server>(main_pool->get_next_loop(),
			msg_dealer, msg_maker);
		m_tcp_server->start(port);

		bool is_detach = false;
		main_pool->run(is_detach);
		task_io_pool->run(is_detach);
	}
	
}