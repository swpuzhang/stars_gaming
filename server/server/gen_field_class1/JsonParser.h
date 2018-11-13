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
	const static Json::value_t JsonType = JsonValue::string;
	const static FieldValue default_value;
};

class IntType
{
public:
	using FieldValue = int;
	const static Json::value_t JsonType = JsonValue::number_integer;
	const static FieldValue default_value;
};

#endif // !JSON_PARESER.H
