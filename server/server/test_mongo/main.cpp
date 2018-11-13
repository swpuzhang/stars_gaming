#include "pch.h"
#include <iostream>
#include "libresource/Mongodb.h"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include "libresource/Redis.h"
#include "libtools/JsonParser.h"
#include "libtools/FiledClass.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

void func()
{
	auto db = MongodbInstance::get_const_instance().get();
	db["test"].insert_one(MGDocument{} << "count" << "1" << MGFinalize);
	mongocxx::options::find_one_and_update opetions;
	opetions.return_document(mongocxx::options::return_document::k_after);
	auto result = db["test"].find_one_and_update({}, MGDocument{} << "$inc" <<
		MGOpenDocument << "counter" << 1 << MGCloseDocument << MGFinalize, opetions);
	auto doc_view = result->view();
	std::cout << "counter:" << doc_view["counter"].get_int32();
}

FIELD_CLASS1(UserAccount, StringType, account);
FIELD_CLASS2(UserAccount2, StringType, account, StringType, device_code);
FIELD_CLASS3(UserAccount3, StringType, account,
	StringType, device_code,
	IntType, user_id
);
FIELD_CLASS6(AccountInfo,
	StringType, account,
	StringType, device_code,
	StringType, device_name,
	IntType, app_type,
	IntType, user_id,
	IntType, login_deal);

int main(int argc, char* argv[]) {

	mongocxx::client conn{ mongocxx::uri{} };
	auto collection = conn["test"]["col"];
	try {
		auto result = collection.insert_one(make_document(kvp("test", 1)));

		if (!result) {
			std::cout << "Unacknowledged write. No id available." << std::endl;
			return EXIT_SUCCESS;
		}
		
		if (result->inserted_id().type() == bsoncxx::type::k_oid) {
			bsoncxx::oid id = result->inserted_id().get_oid().value;
			std::string id_str = id.to_string();
			std::cout << "Inserted id: " << id_str << std::endl;
		}
		else {
			std::cout << "Inserted id was not an OID type" << std::endl;
		}

		mongocxx::database db = conn["test"];
		MGOptions::update op_update;
		op_update.upsert(true);
		auto set_result = db["test"].update_one({}, MGDocument{} << "$setOnInsert"
			<< MGOpenDocument << "counter" << 10000500 << MGCloseDocument << MGFinalize, op_update);
		if (!set_result)
		{
			std::cout << "setinsert error\n";
			return 0;
		}
		
		if (set_result->upserted_id())
		{
			std::cout << set_result->upserted_id()->get_oid().value.to_string() << std::endl;
		}

		std::cout << set_result->matched_count() << std::endl;
		std::cout << set_result->modified_count() << std::endl;

		mongocxx::options::find_one_and_update opetions;
		opetions.return_document(mongocxx::options::return_document::k_after);
		auto update_result = db["test"].find_one_and_update({}, MGDocument{} << "$inc" <<
			MGOpenDocument << "counter" << 1 << MGCloseDocument << MGFinalize, opetions);
		if (!update_result)
		{
			std::cout << "update error\n";
			return 0;
		}
		auto doc_view = update_result->view();
		std::cout << "counter:" << bsoncxx::to_json(doc_view) << std::endl;
	}
	catch (const mongocxx::exception& e) {
		std::cout << "An exception occurred: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
