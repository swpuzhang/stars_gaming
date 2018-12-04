#ifndef LOGIN_MODULE_MANAGER_H
#define LOGIN_MODULE_MANAGER_H


#include "UserCommon.h"
#include <boost/serialization/singleton.hpp>
#include <vector>
#include <map>
#include <chrono>

#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"
#include "libtools/Types.h"
#include "libtools/AsioTypes.h"
#include "libresource/Redis.h"
#include "libtools/JsonParser.h"
#include "libserver/TcpSession.h"

#include "ErrorCode.pb.h"
#include "LoginMsg.pb.h"
#include "LoginCmd.pb.h"

using namespace ErrorCode;
using namespace LoginCmd;
using namespace LoginMsg;

using namespace std::chrono;

constexpr TY_INT64 MAX_LOGIN_SEC = 30;
constexpr char USER_INFO_COLLECT[] = "c_user_info";
constexpr char USER_ID_COLLECT[] = "c_user_id";
constexpr char USER_MONEY_COLLECT[] = "c_user_money";
constexpr char VERSON_COLLECT[] = "c_version";
constexpr int INIT_USER_ID = 10000500;
constexpr int ACCOUNT_EXPIRE_SEC = 7 * 3600 * 24;
constexpr int DEFAULT_SEX = 0;  //男

enum class emAppType 
{
	MOBILE_ANDROID  =  0,
	MPBILE_IPHONE,
	HTME5 ,
	PC_WINDOWS,
	PC_MAC,
};

enum class emUpdateType
{
	NO_NEED_UPDATE = 0,
	NEED_UPDATE,
	FORCE_UPDATE
};

enum class emLoginDeal
{
	REGISTER_ACCOUNT,
	LOGIN_ACCOUNT,
};

FIELD_CLASS3(VersonInfo,
	StringType, cur_version,
	StringType, force_version,
	StringType, update_desc);

class LoginModule
{
public:
	using SELF_TYPE = LoginModule;
	LoginModule() {}
	void open(IoLoop & ioloop);
	void user_session_open(const TcpSessionPtr& user_sessoin);
	void user_session_close(const TcpSessionPtr& user_sessoin);
	void user_login(const TcpMsgPtr& msg);
	void load_version_info();
	
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
	UserInfo generate_new_user(const LoginRequest& login_req, const AccountInfo& account_info);
	
	void notify_user_login(const AccountInfo& account_info, const LoginRequest& login_req);

public:
	static std::vector<std::string> m_lobby_list;
	static	TY_UINT32 m_local_ip;
private:
	std::map<TcpSessionPtr, steady_clock::time_point> m_session_time;  //创建连接的信息
	std::chrono::seconds m_max_login_sec{ MAX_LOGIN_SEC };  //最大登录时间, 超过该时间断开连接
	std::unique_ptr<boost::asio::steady_timer > m_timer;
	
	TY_UINT16 m_seq = 0;

	VersonInfo m_versoin_info;
	const std::vector<std::string> m_default_male_header_urls = { "default_header/default1.png" ,
		"default_header/default2.png","default_header/default3.png" };
	const std::vector<std::string> m_default_female_header_urls = { "default_header/default4.png" ,
		"default_header/default5.png","default_header/default6.png" };
	
};

using LoginInstance = boost::serialization::singleton<LoginModule>;

#endif // !LOGIN_TASK_MANAGER_H