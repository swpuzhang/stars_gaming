#pragma once
#include <string>
#include <vector>
#include <memory>
#include <boost/serialization/singleton.hpp>
#include <boost/asio/signal_set.hpp>
#include "libtools/MultiProcess.h"
#include "libserver/MqInfo.h"
#include "libserver/MqClient.h"
#include "libserver/Server.h"
#include "libtools/JsonParser.h"

class LanchProcess
{
public:
	LanchProcess() {}
	void run(int argc, char *argv[]);
	std::unique_ptr<MqClient> handle_single() {}
	MqClient& get_mq() { return *m_mq_client; }
private:
	bool parse_config();

	std::vector<int> m_ports;
	bool m_is_master = false;
	std::string m_config_file;
	Json m_config_json;
	std::vector<std::string> m_argv;
	MultiProcess m_multi_process;
	std::unique_ptr<boost::asio::signal_set> m_signals;
	std::unique_ptr<MqClient> m_mq_client;
	std::unique_ptr<Server> m_tcp_server;
	std::string m_local_ip;
};

using LanchProcessInstance = boost::serialization::singleton<LanchProcess>;
