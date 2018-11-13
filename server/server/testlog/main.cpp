#include "liblog/Log.h"
#include "libtools/XmlConfig.h"
#include "libtools/JsonParser.h"
#include "libtools/FileTool.h"
#include "libserver/MqInfo.h"
#include <thread>
#include <chrono>


int test_fun()
{
	TRACE_FUNCATION();
	std::string str = "\"a\":1}";
	Json jv = Json::parse(str);
	if (!jv.is_valid())
	{
		DEBUG_LOG << "jv is null";
	}
	return 0;
}

int main(int argc, char *argv[])
{

	Logger::get_mutable_instance().init("./config/config.json");
	DEBUG_LOG << "hello";
	DEBUG_LOG << "world";
	test_fun();
	MqInfo mqinfo;
	mqinfo.parse_config(FileTool::get_file_content("./config/config.json"));
	std::this_thread::sleep_for(std::chrono::seconds(5));
	Logger::get_mutable_instance().safe_close();

	return 0;
}