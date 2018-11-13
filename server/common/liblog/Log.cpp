#include "Log.h"
#include<iostream>  
#include<fstream> 
#include <iostream>
#include <boost/lexical_cast.hpp>
#include<boost/filesystem.hpp> 
#include <boost/date_time/posix_time/posix_time.hpp>

constexpr int PATH_MAX_LEN = 260;

std::string getPorcName(void)
{
#ifdef _WIN32
	char nameBuf[PATH_MAX_LEN] = {0};
	::GetModuleFileName(NULL, nameBuf, PATH_MAX_LEN-1);
	std::string strPathName = nameBuf;
	std::string::size_type pos_begin = strPathName.find_last_of('\\');
	std::string::size_type pos_end = strPathName.find_last_of('.');
	strPathName = strPathName.substr(pos_begin+1, pos_end - pos_begin - 1);

	return strPathName;
#else
	char link[PATH_MAX_LEN] = {0};
	char buf[PATH_MAX_LEN] = {0};
	sprintf(link, "/proc/%d/exe", ::getpid());
	if (readlink(link, buf, PATH_MAX_LEN -1 ) < 0)
	{
		printf("ACE_OS::readlink error=%d\n", errno);
		return "default";
	}

	std::string strPathName = buf; 
	int pos_begin = strPathName.find_last_of('/');
	strPathName = strPathName.substr(pos_begin+1);
	return strPathName;
#endif
}

int CLog::init(const std::string& config_path)
{
	m_proc_name = getPorcName();
	m_config_file_path = config_path;
	if (m_config_file_path.empty())
	{
		m_config_file_path = "./config.json";
	}
	load_log_config();
	return 0;
}


int CLog::load_log_config(void)
{
	std::ifstream ifile(m_config_file_path);
	if (!ifile.is_open())
	{
		set_log_att();
		return 0;
	}
	Json jv;
	ifile >> jv;

	if (!jv.is_object() || !jv["log"].is_object() || !jv["log"]["default"].is_object())
	{
		set_log_att();
		return 0;
	}

	m_log_config.m_level = std::stoul(jv["log"]["default"]["level"].get<std::string>());
	m_log_config.m_max_file_num = std::stoul(jv["log"]["default"]["filenum"].get<std::string>());
	m_log_config.m_max_file_size = std::stoul(jv["log"]["default"]["filesize"].get<std::string>());
	m_log_config.m_log_dir = jv["log"]["default"]["dir"].get<std::string>();

	if (!jv["log"][m_proc_name].is_object())
	{
		set_log_att();
		return 0;
	}


	m_log_config.m_level = std::stoul(jv["log"][m_proc_name]["level"].get<std::string>());
	m_log_config.m_max_file_num = std::stoul(jv["log"][m_proc_name]["filenum"].get<std::string>());
	m_log_config.m_max_file_size = std::stoul(jv["log"][m_proc_name]["filesize"].get<std::string>());
	m_log_config.m_log_dir = jv["log"][m_proc_name]["dir"].get<std::string>();

	std::string cfg_tag = m_proc_name;
#ifdef WIN32
	cfg_tag += "_" + boost::lexical_cast<std::string>(::_getpid());
#else
	cfg_tag += "_" + boost::lexical_cast<std::string>(::getpid());
#endif
	

	if (!jv["log"][cfg_tag].is_object())
	{
		set_log_att();
		return 0;
	}

	m_log_config.m_level = std::stoul(jv["log"][cfg_tag]["level"].get<std::string>());
	m_log_config.m_max_file_num = std::stoul(jv["log"][cfg_tag]["filenum"].get<std::string>());
	m_log_config.m_max_file_size = std::stoul(jv["log"][cfg_tag]["filesize"].get<std::string>());
	m_log_config.m_log_dir = jv["log"][cfg_tag]["dir"].get<std::string>();


	set_log_att(); 
	return 0;
}

bool CLog::set_log_att()
{
	std::string strWorkDir = boost::filesystem::current_path().string();
#ifdef _WIN32
	std::string strLogDir = strWorkDir + "\\log";
	m_log_config.m_log_dir = strLogDir + "\\" + m_proc_name;
#else
	std::string strLogDir = strWorkDir + "/log";
	m_log_config.m_log_dir = strLogDir + "/" + m_proc_name;
#endif
	if (!boost::filesystem::exists(m_log_config.m_log_dir))
	{
		boost::filesystem::create_directories(m_log_config.m_log_dir);
	}

	spdlog::init_thread_pool(100000, 1);
	spdlog::flush_every(std::chrono::seconds(5));
	std::string str_file_name;
#ifdef WIN32
	str_file_name = m_log_config.m_log_dir + "/" + m_proc_name + "_" + TimeTool::get_cur_day() +
		"_p" + boost::lexical_cast<std::string>(::_getpid()) + ".log";
#else
	str_file_name = m_log_config.m_log_dir + "/" + m_proc_name + "_" + TimeTool::get_cur_day() +
		"_p" + boost::lexical_cast<std::string>(::getpid()) + ".log";
#endif
	m_logger = spdlog::rotating_logger_mt<spdlog::async_factory>("logger", str_file_name,
		m_log_config.m_max_file_size * 1024 * 1024, m_log_config.m_max_file_num);
	m_logger->set_level(spdlog::level::level_enum(m_log_config.m_level));
	return true;
}

