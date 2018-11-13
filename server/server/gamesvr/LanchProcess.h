#pragma once
#include <string>
#include <vector>
#include <memory>
#include <boost/serialization/singleton.hpp>
#include <boost/asio/signal_set.hpp>
#include "libtools/MultiProcess.h"
#include "libserver/MqInfo.h"
#include "libserver/MqClient.h"


class LanchProcess
{
public:
	LanchProcess() {}
	void run(int argc, char *argv[]);
	void handle_single() {}
private:
	bool m_is_master = false;
	std::string m_config_file;
	int m_argc = 0;
	std::vector<std::string> m_argv;
	MultiProcess m_multi_process;
	std::unique_ptr<boost::asio::signal_set> m_signals;
	std::unique_ptr<MqClient> m_mq_client;
	
};

using LanchProcessInstance = boost::serialization::singleton<LanchProcess>;