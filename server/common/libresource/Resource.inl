#ifndef RESOURCE_INL
#define  RESOURCE_INL
#include "Resource.h"

#include <iostream>

template<typename T>
Resource<T>::Resource()
{

}

template<typename T>
bool Resource<T>::parse_config(const Json& config_jv)
{
	std::copy(config_jv.begin(), config_jv.end(), std::back_inserter(m_configs));
	return true;
}

template<typename T>
bool Resource<T>::insert_one(std::thread::id threadid)
{
	TRACE_FUNCATION();
	ResourceMap &resources = m_thread_resource[threadid];
	for (auto &e : m_configs)
	{
		std::string config_name = e["config_name"];
		auto &one_resource = resources[config_name];
		m_parse_one_fun(e, one_resource);
	}
	return true;
}

template<typename T>
bool Resource<T>::insert(std::vector< std::thread::id> thread_ids)
{
	for (auto &e : thread_ids)
	{
		insert_one(e);
	}
	return true;
}




#endif