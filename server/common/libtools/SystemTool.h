#ifndef SYSTEM_TOOL
#define SYSTEM_TOOL

#pragma once
#ifdef _WIN32
#include <process.h>
#include <Windows.Applicationmodel.Activation.h>
#else
#include <unistd.h>
#endif // _WIN32
#include <chrono>
#include <iostream>
#include <sstream>

#include "Types.h"

constexpr static int PATH_MAX_LEN = 260;

class SystemTool
{
	
public:
	static std::string get_process_name(void)
	{
#ifdef _WIN32
		char name_buf[PATH_MAX_LEN] = { 0 };
		::GetModuleFileName(NULL, name_buf, PATH_MAX_LEN - 1);
		std::string str_path_name = name_buf;
		std::string::size_type pos_begin = str_path_name.find_last_of('\\');
		std::string::size_type pos_end = str_path_name.find_last_of('.');
		str_path_name = str_path_name.substr(pos_begin + 1, pos_end - pos_begin - 1);

		return str_path_name;
#else
		char link[PATH_MAX_LEN] = { 0 };
		char buf[PATH_MAX_LEN] = { 0 };
		sprintf(link, "/proc/%d/exe", ::getpid());
		if (readlink(link, buf, PATH_MAX_LEN - 1) < 0)
		{
			printf("readlink error=%d\n", errno);
			return "default";
		}

		std::string str_path_name = buf;
		int pos_begin = str_path_name.find_last_of('/');
		str_path_name = str_path_name.substr(pos_begin + 1);
		return str_path_name;
#endif
		
		
	}
	
	static std::string get_object_id(TY_INT32 int_ip, int pid, int seq)
	{
		std::chrono::steady_clock::time_point tnow = std::chrono::steady_clock::now();
		std::stringstream ss;
		ss << std::hex
			<< std::chrono::duration_cast<std::chrono::seconds>(tnow.time_since_epoch()).count()
			<< int_ip << pid << seq;

		return ss.str();
	}

	static int  get_pid()
	{
#ifdef WIN32
		
		return  ::_getpid();
#else
		
		return ::getpid();
	}
#endif

};

#endif // SYSTEM_TOOL
