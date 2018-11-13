#pragma once
#include <string>
#include <thread>
#include <memory>
#include <vector>

class MultiProcess
{
public:
	using SELF_TYPE = MultiProcess;
	void creat_process(const std::string& cmd, int count);
private:
	void create_one_process(const std::string& cmd);
	std::vector<std::shared_ptr<std::thread> > m_threads;
};
