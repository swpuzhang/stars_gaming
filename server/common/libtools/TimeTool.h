#ifndef TIME_TOOL_H
#define TIME_TOOL_H
#include <chrono>
#include <time.h>
#include <ctime>
#include <boost/date_time/gregorian/gregorian.hpp>

class TimeTool
{
public:
	static std::string get_cur_day()
	{
		char buf[128];
		time_t tnow = time(NULL);
		tm tmnow;
#ifdef _WIN32
		localtime_s(&tmnow, &tnow);
#else
		localtime_r(&tnow, &tmnow);
#endif
		sprintf(buf, "%4d%02d%02d%02d%02d%02d", tmnow.tm_year + 1900, tmnow.tm_mon + 1,
			tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min, tmnow.tm_sec);
		return buf;
	}
};

#endif // !TIME_TOOL_H
