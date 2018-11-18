#include <iostream>
#include <fstream>
#include <string>
#include "Json.hpp"
#include "FiledClass.h"
#include "JsonParser.h"
#include "test.h"


void gen_file()
{
	std::ofstream ofile("FiledClass.h");
	for (int i = 1; i <= 20; ++i)
	{
		ofile << "\n#define FIELD_CLASS" << i << "(class_name, ";
		for (int j = 1; j <= i; ++j)
		{
			if (i != j)
			{
				ofile << "type" << j << "," << "field" << j << ",";
			}
			else
			{
				ofile << "type" << j << "," << "field" << j << ") ";
			}
		}
		ofile << "class class_name \\\n{\\\npublic:\\\n";
		for (int j = 1; j <= i; ++j)
		{
			ofile << "std::string filed_##field" << j << "() { return #field" << j << "; }\\\n";
		}

		
		for (int j = 1; j <= i; ++j)
		{
			ofile << "type" << j << "::FieldValue m_##field" << j << " = type" << j << "::default_value ; \\\n";
		}
		ofile << "Json m_jv_temp = {\\\n";
		for (int j = 1; j <= i; ++j)
		{
			if (j != i)
			{

				ofile << "{#field" << j << ", type" << j << "::JsonType } , \\\n";
			}
			else
			{
				ofile << "{#field" << j << ", type" << j << "::JsonType } }; \\\n";
			}
		}

		ofile << "void from_json(const Json& jv) \\\n{\\\n";
		for (int j = 1; j <= i; ++j)
		{
			ofile << "m_##field" << j << " = jv[#field" << j << "].get<type" << j << "::FieldValue>();\\\n";
		}
		ofile << "}\\\nJson to_json() const \\\n{\\\nJson jv; \\\n";
		for (int j = 1; j <= i; ++j)
		{
			ofile << "jv[#field" << j << "] = m_##field" << j << "; \\\n";
		}
		ofile << "return jv; \\\n}\\\n";
		ofile << "}\n";
	}
	ofile.close();
}

FIELD_CLASS1(UserAccount, StringType, account);
FIELD_CLASS2(UserAccount2, StringType, account, StringType, device_code);
FIELD_CLASS3(UserAccount3, StringType, account,
	StringType, device_code,
	IntType, user_id
);



UserInfo generate_new_user(const AccountInfo& account_info)
{
	UserInfo user_info;
	user_info.m_account = account_info.m_account;
	user_info.m_user_id = account_info.m_user_id;
	return user_info;
}

int main()
{

	UserAccount a;
	a.m_account = "1";
	std::cout << a.m_account << std::endl;
	std::cout << a.m_jv_temp << std::endl;

	UserAccount2 a2;
	a2.m_account = "1";
	a2.m_device_code = "2";
	std::cout << a2.m_account << std::endl;
	std::cout << a2.m_device_code << std::endl;
	std::cout << a2.m_jv_temp << std::endl;

	UserAccount3 a3;

	std::cout << a.filed_account() << std::endl;

	a3.from_json(Json{ {"account", "zhangyang" }, {"device_code", "acdroid"}, {"user_id", 10001} });
	std::cout << a3.m_account << std::endl;
	std::cout << a3.m_device_code << std::endl;
	std::cout << a3.m_user_id << std::endl;
	std::cout << a3.m_jv_temp << std::endl;
	Json jv = a3.to_json();
	std::cout << jv << std::endl;
	UserAccount3 a4 = a3;
	Json jv4 = a4.to_json();
	std::cout << jv4 << std::endl;
	UserInfo user_info;
	Json jv_info = user_info.to_json();
	std::cout << jv_info << std::endl;
	AccountInfo acc_info;
	UserInfo user_info2 = generate_new_user(acc_info);
	

	Json jv_info2 = user_info2.to_json();
	user_info2.from_json(jv_info2);
	std::cout << jv_info2 << std::endl;
	gen_file(); 

	Json jv_test{ {"user_id", "1234"}, {"user_name", "zhangyang"}, {"sub",  { {"age", 1}, {"add","shenzheng"} } } };
	std::cout << jv_test << std::endl;

	return 0;
}