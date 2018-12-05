#include "liblog/Log.h"
#include "LanchProcess.h"

int main(int argc, char *argv[])
{

	Logger::get_mutable_instance().init("./config/config.json");
	DEBUG_LOG << "hello world";
	LanchInstance::get_mutable_instance().run("./config/testgameclient.xml", argc, argv);
	Logger::get_mutable_instance().safe_close();
	return 0;
}