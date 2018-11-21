#ifndef RESOURCE_H
#define  RESOURCE_H
#include "liblog/Log.h"
#include "libtools/JsonParser.h"
#include "libtools/FunctionBindMacro.h"
#include "bsoncxx/builder/basic/array.hpp"
#include "bsoncxx/builder/basic/document.hpp"
#include "bsoncxx/builder/basic/kvp.hpp"
#include "bsoncxx/json.hpp"

#include "mongocxx/client.hpp"
#include "mongocxx/instance.hpp"
#include "mongocxx/options/find.hpp"
#include "mongocxx/uri.hpp"

#include "cpp_redis/cpp_redis"

#include <boost/serialization/singleton.hpp>
#include <thread>
#include <memory>
#include <unordered_map>


class MongoOb
{
public:
	std::string m_db_name;
	std::unique_ptr<mongocxx::client> m_client;
};

template<typename T>
class Resource
{
	
public:
	Resource();
	using ResourceMap = std::unordered_map<std::string, std::unique_ptr<T>>;
	using ThreadMap = std::unordered_map<std::thread::id, ResourceMap>;
	bool parse_config(const Json& config_jv);
	bool insert_one(std::thread::id threadid);
	bool insert(std::vector< std::thread::id> thread_ids);
protected:
	std::vector<Json>m_configs;
	ThreadMap m_thread_resource;
	std::function<bool (const Json&, std::unique_ptr<T>&)> m_parse_one_fun;
};
#include "Resource.inl"
#endif // RESOURCE_H
