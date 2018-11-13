#include "XmlConfig.h"
#include <exception>
#include <iostream>
#include <string>
bool XmlConfig::LoadXmlFile(const std::string& filepath)
{
	try
	{
		boost::property_tree::read_xml(filepath, m_pt);
	}
	catch (std::exception &)
	{
		return false;
	}
	return true;
}

std::string XmlConfig::GetItemValue(const std::string& element,const std::string& attribute,const std::string& default_value) const
{
	std::string retString;
	try
	{
		retString = m_pt.get<std::string>("root." + element + "." + attribute, default_value);
	}
	catch (std::exception &)
	{
		retString = default_value;
	}
	return retString;
}

std::string XmlConfig::GetItemString(const std::string& element,const std::string& attribute,const std::string& default_value) const
{
	std::string retString;
	try
	{
		retString = m_pt.get<std::string>("root." + element + "." + attribute, default_value);
	}
	catch (std::exception &)
	{
		retString = default_value;
	}
	return retString;
}

int XmlConfig::GetItemInt(const std::string& element, const std::string& attribute, int default_value) const
{
	int ret;
	try
	{ 
		ret = m_pt.get<int>("root." + element + "." + attribute, default_value);
	}
	catch (std::exception &)
	{
		ret = default_value;
	}
	return ret;
}
