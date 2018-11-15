#ifndef LOGIN_MODULE_MANAGER_H
#define LOGIN_MODULE_MANAGER_H

#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"
#include "libtools/Types.h"
#include "libtools/AsioTypes.h"
#include "libresource/Redis.h"
#include "libtools/JsonParser.h"
#include "libtools/FiledClass.h"

#include <vector>
#include <map>
#include <chrono>


using namespace std::chrono;

constexpr TY_INT64 MAX_LOGIN_SEC = 30;
constexpr char USER_INFO_COLLECT[] = "c_user_info";
constexpr char USER_ID_COLLECT[] = "c_user_id";
constexpr char USER_MONEY_COLLECT[] = "c_user_money";
constexpr int INIT_USER_ID = 10000500;
constexpr int ACCOUNT_EXPIRE_SEC = 7 * 3600 * 24;

enum class emAppType 
{
	MOBILE_ANDROID  =  0,
	MPBILE_IPHONE,
	HTME5 ,
	PC_WINDOWS,
	PC_MAC,
};

enum class emLoginDeal
{
	REGISTER_ACCOUNT,
	LOGIN_ACCOUNT,
};


FIELD_CLASS3(AccountInfo,
	StringType, account,
	IntType, app_type,
	IntType, user_id);

FIELD_CLASS13(UserInfo,
	StringType, account,
	StringType, device_code,
	StringType, device_name,
	IntType, app_type,
	IntType, user_id,
	IntType, user_sex,
	StringType, user_name,
	StringType, header_url,
	IntType, user_level,
	IntType, user_cur_exp,
	IntType, need_exp,
	IntType, last_login_time,
	IntType, last_offline_time);

class LoginModule
{
public:
	using SELF_TYPE = LoginModule;
	LoginModule(IoLoop & ioloop, const std::string local_ip);
	void user_session_open(const TcpSessionPtr& user_sessoin);
	void user_session_close(const TcpSessionPtr& user_sessoin);
	void user_login(const TcpMsgPtr& msg);
private:
	bool insert_new_user(const UserInfo& user_info);
	bool insert_new_user_redis(const UserInfo& user_info);
	bool insert_new_user_db(const UserInfo& user_info);
	std::string user_info_redis_key(int user_id) const;
	std::string account_redis_key(const std::string& account) const;
	bool query_new_userid(int &new_id);
	bool is_need_generate_new_account(const std::string &account, int app_type) const;
	std::string generate_account();
	bool query_user_account(AccountInfo& account_info);
	bool query_from_redis(AccountInfo& account_info);
	bool query_from_mongo(AccountInfo& account_info);
	void on_timeout(const SYSTEM_CODE& err);
	UserInfo generate_new_user(const AccountInfo& account_info);
	std::map<TcpSessionPtr, steady_clock::time_point> m_session_time;  //创建连接的信息
	std::chrono::seconds m_max_login_sec{ MAX_LOGIN_SEC };  //最大登录时间, 超过该时间断开连接
	boost::asio::steady_timer m_timer;
	TY_UINT32 m_local_ip = 0;
	TY_UINT16 m_seq = 0;
};

#endif // !LOGIN_TASK_MANAGER_H