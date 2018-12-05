#pragma once
#include <string>
#include <vector>
#include <boost/serialization/singleton.hpp>
#include "libtools/MultiProcess.h"

class IoLoopPool;

class LanchProcess
{
public:
	LanchProcess(): m_is_master(false){}
	void run(const std::string& config_file, int argc, char *argv[]);
	std::string get_short_filename(const std::string& str);
	const std::shared_ptr<IoLoopPool>& get_io_loop()  const { return m_io_loop; }
	const std::shared_ptr<IoLoopPool>& get_client_loop() const  { return m_client_io_pool; }
private:
	void handle_single() {}
	bool m_is_master;
	std::string m_config_file;
	int m_argc;
	std::vector<std::string> m_argv;
	MultiProcess m_multi_process;
	std::shared_ptr<IoLoopPool> m_io_loop;
	std::shared_ptr<IoLoopPool> m_client_io_pool;
	std::string m_login_host;
};

using LanchInstance = boost::serialization::singleton<LanchProcess>;