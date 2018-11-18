#ifndef MONGODB_H
#define  MONGODB_H
#include "Resource.h"
#include "liblog/Log.h"
#include "libtools/JsonParser.h"
#include "bsoncxx/json.hpp"
#include "bsoncxx/builder/stream/array.hpp"
#include "bsoncxx/builder/stream/document.hpp"
#include "bsoncxx/document/element.hpp"
#include "bsoncxx/document/value.hpp"
#include "bsoncxx/document/view.hpp"
#include "bsoncxx/document/view_or_value.hpp"
#include "bsoncxx/builder/stream/closed_context.hpp"
#include "bsoncxx/builder/stream/helpers.hpp"
#include "bsoncxx/builder/basic/array.hpp"
#include "bsoncxx/builder/basic/document.hpp"
#include "bsoncxx/builder/basic/kvp.hpp"
#include "bsoncxx/json.hpp"

#include "mongocxx/client.hpp"
#include "mongocxx/instance.hpp"
#include "mongocxx/options/find.hpp"
#include "mongocxx/uri.hpp"
#include "mongocxx/collection.hpp"
#include "mongocxx/database.hpp"
#include "bsoncxx/types.hpp"
#include "mongocxx/client.hpp"
#include "mongocxx/exception/exception.hpp"
#include "mongocxx/instance.hpp"

#include <boost/serialization/singleton.hpp>
#include <thread>
#include <memory>
#include <unordered_map>
#include <sstream>

namespace MGOptions = mongocxx::v_noabi::options;
decltype(bsoncxx::stream::close_array)  MGCloseArray =  bsoncxx::stream::close_array;
decltype(bsoncxx::stream::close_document) MGCloseDocument = bsoncxx::stream::close_document;
using  MGDocument = bsoncxx::stream::document;
decltype(bsoncxx::stream::finalize) MGFinalize = bsoncxx::stream::finalize;
decltype(bsoncxx::stream::open_array) MGOpenArray = bsoncxx::stream::open_array;
decltype(bsoncxx::stream::open_document) MGOpenDocument  = bsoncxx::stream::open_document;
using MGResult = bsoncxx::stdx::optional<bsoncxx::document::value>;
using MGDocValue = bsoncxx::document::value;
using BDocument = bsoncxx::types::b_document;
#define  MGKvp bsoncxx::basic::kvp
#define MGMakeDoc bsoncxx::basic::make_document

class Mongodb: public Resource<MongoOb>
{
public:
	using SELF_TYPE = Mongodb;
	Mongodb();
	mongocxx::database get_client(const std::string& map_key = "default") const;
	bool parse_one_resource(const Json& one_jv, std::unique_ptr < MongoOb>& one_resource);
};

typedef boost::serialization::singleton<Mongodb> MongodbInstance;
#define MongoInst MongodbInstance::get_mutable_instance();
#include "Resource.inl"
#endif // RESOURCE_H
