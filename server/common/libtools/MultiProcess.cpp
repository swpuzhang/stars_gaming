#include "MultiProcess.h"
#include "FunctionBindMacro.h"
#include <boost/process.hpp>
#include "liblog/Log.h"
#include <thread>
#include <memory>
#include <vector>
#include <chrono>

namespace bp = boost::process;

void MultiProcess::creat_process(const std::string& cmd, int count)
{
	for (int i = 0; i < count; ++i)
	{
		std::shared_ptr<std::thread> thread(new std::thread(std::bind(&MultiProcess::create_one_process, 
			this, cmd)));
		thread->detach();
		//m_threads.push_back(thread);
	}
	
	//for (auto &e : m_threads)
	//{
	//	e->detach();
	//}
}


void MultiProcess::create_one_process(const std::string& cmd)
{
	while (true)
	{
		bp::child c(cmd);
		std::error_code ec;
		c.wait(ec);
		ERROR_LOG << "pid:" << c.id() << ",cmd:" << cmd << " exit("  << ec << ")";
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}