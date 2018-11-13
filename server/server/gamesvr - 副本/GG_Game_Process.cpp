#include "ggdbi/ggdbi.h"
#include "GG_Game_Process.h"
#include "GG_Game_Factory.h"
#include "libggppss/GG_BaseServer.h"
#include "libggppss/GG_Server.h"
#include "logger/ilog.h"
#include "ace/Process.h"
#include <cassert>
#include "ace/ACE.h"
#include "logger/ilog.h"
#include "ace/Thread_Manager.h"
#include "GG_GameTask_Manager.h"
#include "GG_GameArea_Manager.h"
#include "json/json.h"
#include "GG_Money_Proxy.h"
#include "libggppcc/GG_SiteService.h"
#include "GG_SiteMsg_Factory.h"
#include "GG_BusSiteTask_Manager.h"
#include "ggpp/GG_SiteName.h"
#include "libggppss/GG_BaseServer.h"
#include "GG_RoomMsg.pb.h"
#include "GG_RoomCmd.pb.h"
#include "GG_ErrorCode.pb.h"
#include "libggppcc/GG_MQService.h"
#include "ggpp/GG_SiteName.h"
#include "NativeFeatureIncludesOverrides.h"

#define GG_SQL_QUERY_GAMESVR_PORTS "GG_SQL_QUERY_GAMESVR_PORTS"

GG_Game_Process::GG_Game_Process(void)
	:m_pServer(NULL), m_pService(NULL), m_pGameTaskManager(NULL)
{
}


GG_Game_Process::~GG_Game_Process(void)
{
	GameArea_Manager::instance()->close(1);
	delete m_pServer;
	delete m_pService;
	delete m_pGameTaskManager;
}

int GG_Game_Process::run(const std::string& config_path, int argc, char *argv[])
{
	TRACE_FUNCATION();
	this->parse_share_args(argc, argv);
	if (!m_config.LoadXmlFile(config_path))
	{
		LOG_ERROR("load config failed");
		return -1;
	}

	m_pServer = new GG_Server();
	m_pService = new GG_Service(new GG_Game_Factory, true);
	m_pService->set_service_name("game-service");
	m_pGameTaskManager = new GG_GameTask_Manager(0, 0);
	m_pService->register_task_manager(m_pGameTaskManager);
	m_pServer->register_service(m_pService, false);
	m_pServer->set_heartbeat_interval(m_config.GetItemInt("gamesvr", "heartbeat_interval", "5"));

	ShareClientReactor::instance()->open(m_config.GetItemInt("gamesvr", "share_client_thread", 1));

	int sub_count = m_config.GetItemInt("gamesvr", "subprocess", 0);

	if (is_master_process())
	{
		std::list<uint16_t> all_ports;
		get_port_list(all_ports);
		make_share_port_info(all_ports);
		return this->master_main(sub_count ? 1 : 0, argc, argv);
	}

	return this->subprogram_main(argc, argv);
}

int GG_Game_Process::master_main(int sub_count, int argc, char *argv[])
{
	TRACE_FUNCATION();
	if (sub_count)
	{
		for (std::vector<SharePortInfo>::iterator iter = m_share_ports.begin(); iter != m_share_ports.end(); ++iter)
		{
			std::string cmd = argv[0];
			cmd += " " + toString(1);
			cmd += " " + toString((long)iter->m_share_handle) + " " + toString(iter->m_share_port);	
			LOG_INFO("command_line " + cmd);
			create_subprogram(cmd, 1);
		}
	}
	else
	{
		//assert(m_share_ports.size() <= 1);
		this->subprogram_main(argc, argv);
	}
	return 0;
}

int GG_Game_Process::subprogram_main(int argc, char *argv[])
{
	TRACE_FUNCATION();
	LOG_DEBUG("LIBCAT_SECURITY:" + toString(LIBCAT_SECURITY));
	GG_MQRegisterInfo mqinfo;
	if (!MQService::instance()->load_mqinfo(mqinfo))
	{
		LOG_ERROR("load mqinfo failed");
		return -1;
	}
	setSiteName(m_config.GetItemString("gamesvr", "site_name", "gamesvr"));
	setGameName(m_config.GetItemString("gamesvr", "game_name", "gaple"));
	MQService::instance()->register_task_manager(new GG_BusSiteTask_Manager());
	MQService::instance()->generateNormalKey(getSiteName(), getGameName(),
		toString(this->get_share_port()), mqinfo.m_queue, mqinfo.m_directRoutKey, mqinfo.m_topicRoutKey);
	MQService::instance()->open(mqinfo, new GG_SiteMsg_Factory());
	GameArea_Manager::instance()->open((void*)(long)this->get_share_port());
    m_my_hostname = m_config.GetItemString("gamesvr", "host_ip", "") + ":" + toString(this->get_share_port());
	m_pServer->set_handle(this->get_share_handle());
	GG_BaseServer::CapitalConfig capital;
	capital.m_reactor_count = m_config.GetItemInt("gamesvr", "reactor_count", "10");
	capital.m_thread_per_reactor = m_config.GetItemInt("gamesvr", "reactor_thread", "1");
	capital.m_session_manager_thread = m_config.GetItemInt("gamesvr", "backlog_count", "20");
	capital.m_max_session_count = m_config.GetItemInt("gamesvr", "max_session", 5000);
	capital.m_port = this->get_share_port();
	capital.m_is_encrypt = m_config.GetItemInt("gamesvr", "is_encrypt", 0);
	m_pServer->set_stream_max_recv_speed(m_config.GetItemInt("gamesvr", "max_recv_speed", 20));
	m_pServer->set_stream_max_repeat_seq(m_config.GetItemInt("gamesvr", "max_repeat_seq", 5));
	m_pServer->open(&capital);
	return 0;
}

const CXmlConfig& GG_Game_Process::get_config(void)
{
	return m_config;
}

GG_Service* GG_Game_Process::get_service(void)
{
	assert(NULL != m_pService);
	return m_pService;
}

void GG_Game_Process::get_port_list(std::list<uint16_t> &port_list)
{
	LOG_DEBUG("enter get_port_list");
	GG_DBConnection_Pool::instance()->open("./config/db/db_config_gamesvr.xml");
	std::list<std::string> sql_template;
	sql_template.push_back("./config/db/sql_template_game.json");
	GG_DBTemplate_Manager::instance()->load_template(sql_template);
	
	GG_DBQuery query(GG_SQL_QUERY_GAMESVR_PORTS);
	if (-1 == query.execute(m_config.GetItemString("gamesvr", "host_ip", "") + "%"))
	{
		LOG_ERROR("db query failed");
		LOG_DEBUG("leave get_port_list");
		return;
	}
	while (query())
	{
		int port;
		query >> port;
		port_list.push_back(port);
		LOG_DEBUG("get port:" + toString(port));
	}
	//query.~GG_DBQuery();
	//GG_DBConnection_Pool::instance()->~GG_DBConnection_Pool();
	LOG_DEBUG("leave get_port_list");
	return;
}

tm GG_Game_Process::local_time(time_t t)
{
	t += 3600 * m_config.GetItemInt("gamesvr", "time_zone", 8);
	return *ACE_OS::gmtime(&t);
}

GG_GameTask_Manager* GG_Game_Process::get_game_task_manager(void)
{
	return m_pGameTaskManager;
}

GG_Server* GG_Game_Process::get_server(void)
{
	return m_pServer;
}

std::string GG_Game_Process::get_my_hostname(void)
{
    return m_my_hostname;
}
