#pragma once
#include <string>
#include <vector>
#include <boost/serialization/singleton.hpp>
#include "libtools/MultiProcess.h"
#include "libtools/Types.h"

class IoLoopPool;

class LanchProcess
{
public:
	LanchProcess(): m_is_master(false){}
	void run(const std::string& config_file, int argc, char *argv[]);
	std::string get_short_filename(const std::string& str);
	const std::shared_ptr<IoLoopPool>& get_io_loop()  const { return m_io_loop; }
	const std::shared_ptr<IoLoopPool>& get_client_loop() const  { return m_client_io_pool; }
	const std::pair<std::string, TY_UINT16> get_login_host() const  { return  std::pair<std::string, TY_UINT16>{ m_login_ip, m_login_port }; }
private:
	bool parse_config();
	void handle_single() {}
	bool m_is_master;
	std::string m_config_file;
	Json m_config_json;
	int m_argc;
	std::vector<std::string> m_argv;
	MultiProcess m_multi_process;
	std::shared_ptr<IoLoopPool> m_io_loop;
	std::shared_ptr<IoLoopPool> m_client_io_pool;
	std::string m_login_ip;
	TY_UINT16 m_login_port;
	std::string m_local_ip;
};

using LanchInstance = boost::serialization::singleton<LanchProcess>;