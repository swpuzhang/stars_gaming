#include "Mongodb.h"
#include <iostream>

Mongodb::Mongodb() : Resource<MongoOb>()
{
	mongocxx::instance instance{};
	m_parse_one_fun = MEMFUN_THIS_HOLDER_BIND2(parse_one_resource);
}

 mongocxx::database  Mongodb::get_client(const std::string& map_key) const
{
	auto iter = this->m_thread_resource.find(std::this_thread::get_id());

	auto iter_find = iter->second.find(map_key);
	assert(iter_find != iter->second.end());
	auto &mgclient = *(iter_find->second->m_client);
	return mgclient[iter_find->second->m_db_name];
}

 bool Mongodb::parse_one_resource(const Json& one_jv, std::unique_ptr < MongoOb>& one_resource)
 {
	 TRACE_FUNCATION();
	 one_resource = std::make_unique<MongoOb>();
	 mongocxx::uri  mongo_url(one_jv["url"].get<std::string>());
	 one_resource->m_db_name = one_jv["db_name"].get<std::string>();
	 one_resource->m_client = std::make_unique<mongocxx::client>(mongo_url);
	 return true;
 }
