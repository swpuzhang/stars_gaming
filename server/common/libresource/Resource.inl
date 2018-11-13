#ifndef RESOURCE_INL
#define  RESOURCE_INL
#include "Resource.h"

#include <iostream>

template<typename T>
Resource<T>::Resource()
{

}

template<typename T>
bool Resource<T>::parse_config(const std::string& key_name, const std::string& config_str)
{
	TRACE_FUNCATION();
	auto jv = Json::parse(config_str);
	if (!jv.is_valid())
	{
		ERROR_LOG << "error json" << config_str;
		return false;
	}
	auto jv_mongo = jv[key_name];
	std::copy(jv_mongo.begin(), jv_mongo.end(), std::back_inserter(m_configs));
	return true;
}

template<typename T>
bool Resource<T>::insert(std::thread::id threadid)
{
	TRACE_FUNCATION();
	ResourceMap &resources = m_thread_resource[threadid];
	for (auto &e : m_configs)
	{
		std::string config_name = e["config_name"].get<std::string>();
		auto &one_resource = resources[config_name];
		m_parse_one_fun(e, one_resource);
	}
	return true;
}




#endif