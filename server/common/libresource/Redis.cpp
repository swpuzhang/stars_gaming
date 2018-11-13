#include "Redis.h"
#include "liblog/Log.h"
#include "libtools/Types.h"
#include <iostream>
Redis::Redis()
{
	m_parse_one_fun = MEMFUN_THIS_HOLDER_BIND2(parse_one_resource);
}

RedisClient& Redis::get(const std::string& map_key)
{
	auto threadid = std::this_thread::get_id();
	auto iter = m_thread_resource.find(threadid);
	if (map_key.empty())
	{
		return *(iter->second.begin()->second);
	}
	auto iter_find = iter->second.find(map_key);
	return *(iter_find->second);
}
Json Redis::pair_array_to_json(const cpp_redis::reply& reply, const Json& jv_temp)
{
	Json jv_out;
	if (jv_temp.is_valid() || !jv_temp.is_object())
	{
		ERROR_LOG << "jv_temp is error";
		return jv_out;
	}
	auto & ar = reply.as_array();
	for (size_t i = 0; i < ar.size(); i += 2)
	{
		
		if (!ar[i].is_string())
		{
			WARN_LOG << "redis array[" << i << "] is not string";
			continue;
		}

		if (!ar[i + 1].is_string())
		{
			WARN_LOG << "redis array[" << i + 1 << "] is not string";
			continue;
		}
		auto str_key = ar[i].as_string();
		auto str_value = ar[i].as_string();
		auto& jv_key = jv_temp[str_key];
		switch (jv_key.type())
		{
		case JsonValue::string:
			jv_out[str_key] = str_value;
			break;
		case JsonValue::number_integer:
			jv_out[str_key] = std::atoll(str_value.c_str());
			break;
		case JsonValue::number_unsigned:
			jv_out[str_key] = static_cast<TY_UINT64>(std::atoll(str_value.c_str()));
			break;
		default:
			ERROR_LOG << "jv_temp error: key:" << str_key << " type:" << jv_key.type_name();
			break;
		}
	}
	return jv_out;
}

PairArray Redis::get_pair_array(const cpp_redis::reply& reply )
{
	PairArray result;
	auto & ar = reply.as_array();
	for (size_t i = 0; i < ar.size(); i += 2)
	{
		if (!ar[i].is_string())
		{
			WARN_LOG << "redis array[" << i << "] is not string";
			continue;
		}

		if (!ar[i + 1].is_string())
		{
			WARN_LOG << "redis array[" << i + 1<< "] is not string";
			continue;
		}

		result.m_array[ar[i].as_string()] = ar[i + 1].as_string();
	}
	return result;
}

bool Redis::parse_one_resource(const Json& one_jv, std::unique_ptr<cpp_redis::client>& one_resource)
{
	TRACE_FUNCATION();
	one_resource = std::make_unique<cpp_redis::client>();
	std::string redis_ip = one_jv["ip"].get<std::string>();
	int redis_port = one_jv["port"].get<int>();
	one_resource->connect(redis_ip, redis_port, [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status)
	{
		if (status == cpp_redis::client::connect_state::dropped)
		{
			ERROR_LOG << "client disconnected from " << host << ":" << port;
		}
	}, 0, -1, 1);
	return true;
}