#pragma once
#ifdef _WIN32
#include <process.h>
#include <Windows.Applicationmodel.Activation.h>
#else
#include <unistd.h>
#endif // _WIN32

#include <boost/date_time/posix_time/posix_time.hpp> 
#include <boost/serialization/singleton.hpp>
#include <boost/lexical_cast.hpp>
#include <unordered_map>
#include <string>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/async.h"
#include "spdlog/common.h"

#include "libtools/XmlConfig.h"
#include "libtools/TimeTool.h"
#include "libtools/FileTool.h"
#include "libtools/JsonParser.h"


class CFunLogHelper;

struct LogConfig
{
	size_t m_level;
	size_t m_max_file_size;
	size_t m_max_file_num;
	std::string m_log_dir;
	LogConfig(void) :
		m_level(spdlog::level::trace),
		m_max_file_size(1024 * 1024 * 10),
		m_max_file_num(100),
		m_log_dir("./log")
	{
	}
};

class CLog
{
public:
	int init(const std::string& config_path = "");
	void safe_close() { spdlog::shutdown(); }
	spdlog::logger& get_logger() {return  *m_logger;}
	int get_current_log_level() const { return m_log_config.m_level; }
private:
	int load_log_config(void);
	bool set_log_att();
	//std::string get_short_filename(const std::string& str);
	std::string m_config_file_path;
	std::string m_proc_name;
	LogConfig m_log_config;
	std::shared_ptr<spdlog::logger> m_logger;
};
typedef boost::serialization::singleton<CLog> Logger;

class CFunLogHelper
{
public:
	CFunLogHelper(int line, const std::string& fileName, const std::string& funcName)
		: m_line(line), m_filename(fileName), m_func_name(funcName), m_enter_time(boost::posix_time::microsec_clock::universal_time())
	{

		Logger::get_mutable_instance().get_logger().log(spdlog::level::trace,
			("->" + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(m_filename), funcName, line);
	}
	~CFunLogHelper()
	{
		long long use_milli = (boost::posix_time::microsec_clock::universal_time() - m_enter_time).total_milliseconds();
		Logger::get_mutable_instance().get_logger().log(spdlog::level::trace,
			("<-[" + boost::lexical_cast<std::string>(use_milli) + "ms]" + std::string("[{}:{}:{}]")).c_str(), 
			FileTool::get_short_filename(m_filename), m_func_name, m_line);

	}
private:
	int m_line;
	std::string m_filename;
	std::string m_func_name;
	boost::posix_time::ptime m_enter_time;
};

#define LOG_TRACE(str) Logger::get_mutable_instance().get_logger().trace((str + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(__FILE__), __FUNCTION__, __LINE__)
#define LOG_DEBUG(str) Logger::get_mutable_instance().get_logger().debug((str + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(__FILE__), __FUNCTION__, __LINE__)
#define LOG_INFO(str) Logger::get_mutable_instance().get_logger().info((str + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(__FILE__), __FUNCTION__, __LINE__)
#define LOG_WARN(str) Logger::get_mutable_instance().get_logger().warn((str + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(__FILE__), __FUNCTION__, __LINE__)
#define LOG_ERROR(str) Logger::get_mutable_instance().get_logger().error((str + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(__FILE__), __FUNCTION__, __LINE__)
#define LOG_FATAL(str) Logger::get_mutable_instance().get_logger().critical((str + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(__FILE__), __FUNCTION__, __LINE__)
#define TRACE_FUNCATION() CFunLogHelper create_fun(__LINE__, __FILE__, __FUNCTION__)


#include <sstream>
#include <string>

class LogStream                       
{
public:
	LogStream(int level) : m_level(level) {}
	template<typename T>
	LogStream& operator << (const T& value)
	{
		if (Logger::get_mutable_instance().get_current_log_level() <= m_level)
		{
			m_strstrm << value;
		}
		return *this;
	}
	std::string GetString()
	{
		return m_strstrm.str();
	}
private:
	std::ostringstream m_strstrm;
	int m_level;
};


class LogOpearator
{
public:
	LogOpearator(int level, int linenum, const std::string& filename, const std::string functionname) : m_logstrm(level),
		m_level(level), m_linenum(linenum), m_filename(filename), m_functionname(functionname) {}
	LogStream& stream() { return m_logstrm; }
	~LogOpearator()
	{
		if (Logger::get_mutable_instance().get_current_log_level() <= m_level)
		{
			std::string str = m_logstrm.GetString();
			if (!str.empty())
			{
				Logger::get_mutable_instance().get_logger().log(spdlog::level::level_enum(m_level),
					(str + std::string("[{}:{}:{}]")).c_str(), FileTool::get_short_filename(m_filename), m_functionname, m_linenum);
			}
		}
	}
private:
	LogStream m_logstrm;
	int m_level;
	int m_linenum;
	std::string m_filename;
	std::string m_functionname;
};

#define TRACE_LOG \
	LogOpearator(spdlog::level::trace, __LINE__, __FILE__, __FUNCTION__).stream()

#define DEBUG_LOG \
	LogOpearator(spdlog::level::debug, __LINE__, __FILE__, __FUNCTION__).stream()

#define INFO_LOG \
	LogOpearator(spdlog::level::info, __LINE__, __FILE__, __FUNCTION__).stream()

#define WARN_LOG \
	LogOpearator(spdlog::level::warn, __LINE__, __FILE__, __FUNCTION__).stream()

#define ERROR_LOG \
	LogOpearator(spdlog::level::err, __LINE__, __FILE__, __FUNCTION__).stream()

#define CRITICAL_LOG  LogOpearator(spdlog::level::critical, __LINE__, __FILE__, __FUNCTION__).stream()