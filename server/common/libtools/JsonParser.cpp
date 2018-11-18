#include "JsonParser.h"
const StringType::FieldValue StringType::default_value = "";
const IntType::FieldValue IntType::default_value = 0;
const UIntType::FieldValue UIntType::default_value = 0;
const Int64Type::FieldValue Int64Type::default_value = 0;
const UInt64Type::FieldValue UInt64Type::default_value = 0;

const JsonValue StringType::JsonType = JsonValue::string;
const JsonValue IntType::JsonType = JsonValue::number_integer;
const JsonValue UIntType::JsonType = JsonValue::number_unsigned;
const JsonValue Int64Type::JsonType = JsonValue::number_integer;
const JsonValue UInt64Type::JsonType = JsonValue::number_unsigned;