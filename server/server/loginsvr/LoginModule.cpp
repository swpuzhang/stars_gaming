#include <functional>

#include "libtools/FunctionBindMacro.h"
#include "libtools/JsonParser.h"
#include "libtools/SystemTool.h"
#include "libresource/Mongodb.h"
#include "libresource/Redis.h"
#include "LoginModule.h"
#include "libmessage/Message.h"
#include "ErrorCode.pb.h"
#include "HallMsg.pb.h"
#include "HallCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"
#include "libserver/TcpSession.h"
#include "LoginMsg.pb.h"
#include "LoginCmd.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace HallCmd;
using namespace HallMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;
using namespace LoginCmd;
using namespace LoginMsg;

bool LoginModule::is_need_generate_new_account(const std::string &account,int app_type) const
{
	return  account.empty() && 
		(app_type ==  static_cast<int>(emAppType::MOBILE_ANDROID) ||
			app_type == static_cast<int>(emAppType::MPBILE_IPHONE));
}

LoginModule::LoginModule(IoLoop & ioloop, const std::string local_ip) : 
	m_timer(ioloop.get_io_context()),
	m_local_ip(boost::asio::ip::address_v4::from_string(local_ip).to_uint())
{
	TRACE_FUNCATION();
	m_timer.expires_from_now(m_max_login_sec);
	m_timer.async_wait(MEMFUN_THIS_BIND1(on_timeout, PLACEHOLDER::_1));
}

void LoginModule::user_login(const TcpMsgPtr& msg)
{
	//���APPû�в��ҵ��˺��ļ�, ����û�л�ȡ���豸��,
	//��ô��¼ʱ�᷵��һ��objectid(ʱ���+ip+���̺�+������)��Ϊ�˺�,
	//APP�Ὣ����˺ű����������ļ�
	//�û���¼,APP��ȡ�����ļ���ȡ�˺���Ϣ,���߻�ȡ�豸��Ϣ,
	//��¼��ʱ���˺ź��豸�Ŷ�������,�����ѯ�����ݿ���,��ô�������,����ע���˺�
	const LoginRequest& login_req = msg->pbmessage<LoginRequest>();
	AccountInfo account_info;
	account_info.m_account = login_req.account();
	account_info.m_app_type = login_req.app_type();
	emLoginDeal login_deal = emLoginDeal::LOGIN_ACCOUNT;
	if (is_need_generate_new_account(account_info.m_account, account_info.m_app_type))
	{
		account_info.m_account = generate_account();
		login_deal = emLoginDeal::REGISTER_ACCOUNT;
	}

	if (account_info.m_account.empty())
	{
		msg->send_failed_reason(CODE_ACOCUNT_EMPTY);
		return;
	}
	if (!query_user_account(account_info))
	{
		login_deal = emLoginDeal::REGISTER_ACCOUNT;
	}
	UserInfo user_info;
	if (login_deal == emLoginDeal::REGISTER_ACCOUNT)
	{
		//����һ���µ�uid
		int new_id = 0;
		if (!query_new_userid(new_id))
		{
			msg->send_failed_reason(CODE_SYSTEM_ERROR);
			return;
		}
		//����һ�������, �����Ϣ.
		account_info.m_user_id = new_id;
		RedisInstance::get_mutable_instance().json_to_hash(account_info.to_json(),
			account_redis_key(account_info.m_account), ACCOUNT_EXPIRE_SEC);
		user_info =  generate_new_user(account_info);
		insert_new_user(user_info);

	}

	else
	{
		//��ѯ

	}
	
}

UserInfo LoginModule::generate_new_user(const AccountInfo& account_info)
{
	UserInfo user_info;
	user_info.m_account = account_info.m_account;
	user_info.m_user_id = account_info.m_user_id;
	return user_info;
}

bool LoginModule::insert_new_user(const UserInfo& user_info)
{
	if (!insert_new_user_db(user_info))
	{
		return false;
	}
	insert_new_user_redis(user_info);
}

bool LoginModule::insert_new_user_redis(const UserInfo& user_info)
{
	cpp_redis::client& redis_client = RedisInstance::get_mutable_instance().get_client();
	RedisInstance::get_mutable_instance().json_to_hash(user_info.to_json(), 
		user_info_redis_key(user_info.m_user_id), ACCOUNT_EXPIRE_SEC);
	return true;
}

bool LoginModule::insert_new_user_db(const UserInfo& user_info)
{
	mongocxx::database db = MongodbInstance::get_mutable_instance() .get_client();
	mongocxx::collection  collect = db[USER_INFO_COLLECT];
	MGOptions::update op_update;
	op_update.upsert(true);
	auto set_result = collect.update_one(MGDocument{ } << "user_id" << user_info.m_user_id << MGFinalize, MGDocument{} << "$set"
		<< MGOpenDocument << "counter" << INIT_USER_ID << MGCloseDocument << MGFinalize, op_update);
	if (!set_result)
	{
		return false;
	}
	return true;
}

bool LoginModule::query_new_userid(int &new_id)
{
	mongocxx::database db = MongodbInstance::get_mutable_instance().get_client();
	mongocxx::collection  collect = db[USER_ID_COLLECT];
	/*MGOptions::update op_update;
	op_update.upsert(true);
	auto set_result = collect.update_one({}, MGDocument{} << "$setOnInsert"
		<< MGOpenDocument << "counter" << INIT_USER_ID << MGCloseDocument << MGFinalize, op_update);
	if (!set_result)
	{
		return false;
	}*/

	MGOptions::find_one_and_update options;
	options.return_document(MGOptions::return_document::k_after);
	auto result = collect.find_one_and_update({}, MGDocument{} << "$inc" << 
		MGOpenDocument << "counter" << 1 << MGCloseDocument << MGFinalize,
		options);
	if (!result)
	{
		return false;
	}
	auto doc_view = result->view();
	new_id = doc_view["counter"].get_int32();
	return true;
}

void LoginModule::on_timeout(const SYSTEM_CODE& err)
{
	steady_clock::time_point tnow = steady_clock::now();
	for (auto iter = m_session_time.begin(); iter != m_session_time.end();)
	{
		if (tnow < iter->second + m_max_login_sec)
		{
			++iter;
			continue;
		}
		iter = m_session_time.erase(iter);
	}
}

void LoginModule::user_session_open(const TcpSessionPtr& user_sessoin)
{
	m_session_time[user_sessoin] = steady_clock::now();
}

void LoginModule::user_session_close(const TcpSessionPtr& user_sessoin)
{
	m_session_time.erase(user_sessoin);
}

std::string LoginModule::generate_account()
{
	return SystemTool::get_object_id(m_local_ip, SystemTool::get_pid(), m_seq++);
}


bool LoginModule::query_user_account(AccountInfo& account_info)
{
	if (query_from_redis(account_info) || query_from_mongo(account_info))
	{
		RedisInstance::get_mutable_instance().json_to_hash(account_info.to_json(), 
			account_redis_key(account_info.m_account), ACCOUNT_EXPIRE_SEC);
		return true;
	}
	return false;
}

bool LoginModule::query_from_mongo(AccountInfo& account_info)
{
	mongocxx::database db = MongodbInstance::get_mutable_instance().get_client();
	auto  collect = db[USER_INFO_COLLECT];
	auto result = collect.find_one(MGDocument{} << "user_account"
			<< account_info.m_account << MGFinalize);
	if (!result)
	{
		ERROR_LOG << "mongodb result:" << bsoncxx::to_json(*result);
		return false;
	}
	Json jv = Json::parse( bsoncxx::to_json(*result));
	account_info.from_json(jv);
	return true;
}

std::string LoginModule::user_info_redis_key(int user_id) const
{
	return std::to_string(user_id) + "|user_info";
}

std::string LoginModule::account_redis_key(const std::string& account) const
{
	return account + "|account";
}

bool LoginModule::query_from_redis(AccountInfo& account_info)
{
		auto& redis_client = RedisInstance::get_mutable_instance().get_client();
		auto fr = redis_client.hgetall(account_redis_key(account_info.m_account));
		redis_client.commit();
		cpp_redis::reply r = fr.get();
		if (r.is_null() || r.is_error())
		{
			WARN_LOG << "account:" << account_info.m_account << " login redis null";
			return false;
		}
		Json result = RedisInstance::get_mutable_instance().pair_array_to_json(r, account_info.m_jv_temp);
		account_info.from_json(result);
		return true;
}
