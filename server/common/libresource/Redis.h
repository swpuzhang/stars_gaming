#ifndef REIDS_H
#define  REIDS_H
#include "Resource.h"
#include "liblog/Log.h"
#include "libtools/JsonParser.h"
#include "libtools/Types.h"
#include "cpp_redis/cpp_redis"
#include <boost/serialization/singleton.hpp>
#include <thread>
#include <memory>
#include <unordered_map>
#include <sstream>

using RedisClient = cpp_redis::client;


class PairArray
{
public:
	std::unordered_map<std::string, std::string> m_array;
	std::string get_string(const std::string & key)
	{
		auto iter = m_array.find(key);
		if (iter == m_array.end())
		{
			return std::string("");
		}
		return iter->second;
	}


	template<typename T = int>
	T get_integer(const std::string & key, const T& default_value = 0)
	{
		auto iter = m_array.find(key);
		if (iter == m_array.end())
		{
			return default_value;
		}
		std::stringstream strstr;
		strstr << iter->second;
		T ret = default_value;
		strstr >> ret;
		return ret;
	}

};

class Redis : public Resource<RedisClient>
{
public:

	using SELF_TYPE = Redis;
	Redis();
	bool parse_one_resource(const Json& one_jv, std::unique_ptr<cpp_redis::client>& one_resource);
	RedisClient& get_client(const std::string& map_key = "default");
	PairArray get_pair_array(const cpp_redis::reply& reply);
	Json pair_array_to_json(const cpp_redis::reply& reply, const Json& jv_temp);
	void json_to_hash(const Json& jv, const std::string& key_name, TY_UINT64 expire_sec, const std::string& redis_name = "");
};

typedef boost::serialization::singleton<Redis> RedisInstance;
const static  Redis& RedisInst =  RedisInstance::get_mutable_instance();
#endif
