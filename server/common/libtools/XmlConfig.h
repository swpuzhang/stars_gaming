#pragma once
#include <string>
#include <iostream> 
#include <boost/property_tree/ptree.hpp>  
#include <boost/property_tree/xml_parser.hpp>  
#include <boost/typeof/typeof.hpp>  


class XmlConfig
{
public:
	bool LoadXmlFile(const std::string& filepath);

	std::string GetItemValue(const std::string& element,const std::string& attribute,const std::string& default_value) const;

	int GetItemInt(const std::string& element, const std::string& attribute, int default_value) const;

	std::string GetItemString(const std::string& element,const std::string& attribute,const std::string& default_value) const;

private:
	boost::property_tree::ptree m_pt;  
};