#ifndef JSON_PARESER_H
#define JSON_PARESER_H

#include "Json.hpp"
#include <string>

using Json = nlohmann::json;
using JsonValue = Json::value_t;

class StringType
{
public:
	using FieldValue = std::string;
	const static JsonValue JsonType;
	const static FieldValue default_value;
};

class IntType
{
public:
	using FieldValue = std::int32_t;
	const static JsonValue JsonType;
	const static FieldValue default_value;
};

class UIntType
{
public:
	using FieldValue = std::uint32_t;
	const static JsonValue JsonType;
	const static FieldValue default_value;
};


class Int64Type
{
public:
	using FieldValue = std::int64_t;
	const static JsonValue JsonType;
	const static FieldValue default_value;
};

class UInt64Type
{
public:
	using FieldValue = std::uint64_t;
	const static JsonValue JsonType;
	const static FieldValue default_value;
};

#endif // !JSON_PARESER.H
