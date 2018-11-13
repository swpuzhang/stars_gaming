#pragma once
#include <string>
#include <vector>
#include <memory>
#include <boost/serialization/singleton.hpp>
#include <boost/asio/signal_set.hpp>
#include "libtools/MultiProcess.h"


class LanchProcess
{
public:
	LanchProcess() {}
	void run(const std::string& config_file, int argc, char *argv[]);
	void handle_single() {}
private:
	bool m_is_master;
	std::string m_config_file;
	int m_argc;
	std::vector<std::string> m_argv;
	MultiProcess m_multi_process;
	std::unique_ptr<boost::asio::signal_set> m_signals;
};

using LanchProcessInstance = boost::serialization::singleton<LanchProcess>;