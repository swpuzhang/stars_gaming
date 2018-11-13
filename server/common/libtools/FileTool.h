#ifndef FILE_OP_H
#define FILE_OP_H
#include <string>
#include <fstream>
#include<streambuf>
#include<sstream>

class FileTool
{
public:
	static std::string get_short_filename(const std::string& str)
	{
		std::string short_name = str;
#ifdef _WIN32
		size_t index = short_name.rfind('\\');
#else
		size_t index = short_name.rfind('/');
#endif
		if (index == std::string::npos)
		{
			index = 0;
		}
		else
		{
			++index;
		}
		short_name = short_name.substr(index);
		return short_name;
	}

	static  std::string get_file_content(const std::string& file_path)
	{
		std::ifstream ifile(file_path);
		if (!ifile.is_open())
		{
			return "";
		}
		return std::string ((std::istreambuf_iterator<char>(ifile)),
			std::istreambuf_iterator<char>());
	}
};

#endif // FILE_OP_H
