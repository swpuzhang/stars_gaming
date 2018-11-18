#include "Redis.h"
#include "liblog/Log.h"
#include "libtools/Types.h"
#include <iostream>
Redis::Redis()
{
	m_parse_one_fun = MEMFUN_THIS_HOLDER_BIND2(parse_one_resource);
}

RedisClient& Redis::get_client(const std::string& map_key)
{
	auto threadid = std::this_thread::get_id();
	auto iter = m_thread_resource.find(threadid);
	assert(iter != m_thread_resource.end());
	auto iter_find = iter->second.find(map_key);
	assert(iter_find != iter->second.end());
	return *(iter_find->second);
}

void Redis::json_to_hash(const Json& jv, const std::string& key_name, TY_UINT64 expire_sec, const std::string& redis_name)
{
	RedisClient& rclient = get_client(redis_name);
	for (auto iter = jv.begin(); iter != jv.end(); ++iter)
	{
		auto& jv_key = iter.key();
		auto& jv_value = iter.value();
		switch (jv_value.type())
		{
		case JsonValue::string:
			rclient.hset(key_name, jv_key, jv_value.get<std::string>(), 
				[key_name, jv_key, jv_value](cpp_redis::reply& reply) {
				if (!reply)
				{
					ERROR_LOG << "hset " << key_name << " " << jv_key << " " << jv_value.get<std::string>() << " failed";
				}
			}).commit();
			break;
		case JsonValue::number_integer:
			rclient.hset(key_name, jv_key, std::to_string( jv_value.get<TY_INT64>()), 
				[key_name, jv_key, jv_value](cpp_redis::reply& reply) {
			if (!reply)
			{
					ERROR_LOG << "hset " << key_name << " " << jv_key << " " << jv_value.get<TY_INT64>() << " failed";
			}}).commit();
			break;
		case JsonValue::number_unsigned:
				rclient.hset(key_name, jv_key, std::to_string(jv_value.get<TY_UINT64>()),
					[key_name, jv_key, jv_value](cpp_redis::reply& reply) {
				if (!reply)
				{
					ERROR_LOG << "hset " << key_name << " " << jv_key << " " << jv_value.get<TY_UINT64>() << " failed";
				}}).commit();
			break;
		default:
			ERROR_LOG << "jv_temp error: key:" << key_name << " type:" << jv_value.type_name() << " failed";
			break;
		}
	}
	if (expire_sec != 0)
	{
		rclient.expire(key_name, expire_sec, [key_name, expire_sec](cpp_redis::reply& reply) {
			if (!reply)
			{
				ERROR_LOG << "expire " << key_name << " " << expire_sec << " failed";
			}}).commit();
	}
	
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
		auto& jv_value = jv_temp[str_key];
		switch (jv_value.type())
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
			ERROR_LOG << "jv_temp error: key:" << str_key << " type:" << jv_value.type_name();
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
	std::string redis_ip = one_jv["ip"];
	int redis_port = atoi(one_jv["port"].get<std::string>().c_str());
	one_resource->connect(redis_ip, redis_port, [](const std::string& host, std::size_t port, cpp_redis::client::connect_state status)
	{
		if (status == cpp_redis::client::connect_state::dropped)
		{
			ERROR_LOG << "client disconnected from " << host << ":" << port;
		}
	}, 0, -1, 1);
	return true;
}