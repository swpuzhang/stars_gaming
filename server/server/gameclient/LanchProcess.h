#pragma once
#include <string>
#include <vector>
#include <boost/serialization/singleton.hpp>
#include "libtools/MultiProcess.h"


class LanchProcess
{
public:
	LanchProcess(): m_is_master(false){}
	void run(const std::string& config_file, int argc, char *argv[]);
	std::string get_short_filename(const std::string& str);
private:
	void handle_single() {}
	bool m_is_master;
	std::string m_config_file;
	int m_argc;
	std::vector<std::string> m_argv;
	MultiProcess m_multi_process;
};

using LanchProcessInstance = boost::serialization::singleton<LanchProcess>;