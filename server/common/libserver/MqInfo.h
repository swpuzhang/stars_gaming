#ifndef MQ_INFO_H
#define MQ_INFO_H
#include <string>
#include <vector>
#include <unordered_map>
#include "amqpcpp.h"
#include "libtools/JsonParser.h"
#include "libtools/SystemTool.h"
#include "liblog/Log.h"

class MqInfo
{
public:
	bool parse_config(const std::string& str, const std::string& sub_str)
	{
		Json jv = Json::parse(str);
		if (!jv.is_valid() || !jv.is_object())
		{
			ERROR_LOG << str << "is not a valid json config";
			return false;
		}
		std::string proc_name = SystemTool::get_process_name();
		if (!jv["mq"].is_valid())
		{
			ERROR_LOG << str << "is not a valid json config";
			return false;
		}

		const Json& jvmq = jv["mq"];

		if (!jvmq["default"].is_valid())
		{
			ERROR_LOG << str << "is not a valid json config";
			return false;
		}
		const Json& jvmq_default = jvmq["default"];
		m_ip = jvmq_default["ip"].get<std::string>();
		m_port = std::stoi(jvmq_default["port"].get<std::string>());
		m_login = AMQP::Login(jvmq_default["user"].get<std::string>(),
			jvmq_default["passwd"].get<std::string>());
		m_work_dir = jvmq_default["dir"].get<std::string>();

		if (!jvmq[proc_name].is_valid())
		{
			ERROR_LOG << str << "is not a valid json config";
			return false;
		}

		const Json& jvmq_proc = jvmq[proc_name];
		m_queue = jvmq_proc["queue"].get<std::string>();
		if (!sub_str.empty())
		{
			m_queue += ("-" + sub_str);
		}
		for (const Json &e : jvmq_proc["binds"])
		{
			std::string exchange_name = e["exchange"].get<std::string>();
			if (exchange_name.find(".direct") != std::string::npos)
			{
				INFO_LOG << "exchange_name:" << exchange_name;
				m_exchanges.insert({ exchange_name , AMQP::direct });
			}
			else if (exchange_name.find(".topic") != std::string::npos)
			{
				m_exchanges.insert({ exchange_name , AMQP::topic });
				INFO_LOG << "exchange_name:" << exchange_name;
			}
			else
			{
				ERROR_LOG << str << " amqp type error";
				continue;
			}
			std::vector<std::string> all_rout;
			for (const Json &rout : e["routkeys"])
			{
				std::string one_rout = rout.get<std::string>();

				if (exchange_name.find(".direct") != std::string::npos)
				{
					if (!sub_str.empty())
					{
						one_rout += ("-" + sub_str);
					}
				}
				all_rout.push_back(one_rout);
			}
			m_binds.insert({ exchange_name , all_rout });
		}
		return true;
	}
	MqInfo() : m_port(0) {}
	std::string m_ip;
	int m_port;
	AMQP::Login m_login; 
	std::string m_work_dir;
	std::string m_queue;
	std::unordered_map<std::string, int> m_exchanges;
	std::unordered_map<std::string, std::vector<std::string>> m_binds;
};

#endif // MQ_INFO_H
