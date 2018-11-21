#include <functional>
#include <random>
#include <chrono>

#include "LoginModule.h"
#include "LanchProcess.h"
#include "MqRoute.h"
#include "libtools/FunctionBindMacro.h"
#include "libtools/JsonParser.h"
#include "libtools/SystemTool.h"
#include "libresource/Mongodb.h"
#include "libresource/Redis.h"
#include "libtools/BaseTool.h"

#include "libmessage/Message.h"


std::vector<std::string> LoginModule::m_hall_list = {};
TY_UINT32 LoginModule::m_local_ip = 0;

void LoginModule::load_version_info()
{
	mongocxx::database db = MongodbInstance::get_mutable_instance().get_client();
	mongocxx::collection  collect = db[VERSON_COLLECT];
	mongocxx::options::find find_opt;
	find_opt.sort(MGDocument{} << "cur_version" << -1 << MGFinalize);
	find_opt.limit(1);
	mongocxx::cursor find_result = collect.find({}, find_opt);
	for (auto&& doc : find_result)
	{
		m_versoin_info.from_json(Json::parse(bsoncxx::to_json(doc)));
	}
}

bool LoginModule::is_need_generate_new_account(const std::string &account,int app_type) const
{
	return  account.empty() && 
		(app_type ==  static_cast<int>(emAppType::MOBILE_ANDROID) ||
			app_type == static_cast<int>(emAppType::MPBILE_IPHONE));
}

void LoginModule::open(IoLoop & ioloop)
{
	//启动定时器检查客户端session是否连接过长
	m_timer = std::make_unique<boost::asio::steady_timer >(ioloop.get_io_context());
	m_timer->expires_from_now(m_max_login_sec);
	m_timer->async_wait(MEMFUN_THIS_BIND1(on_timeout, PLACEHOLDER::_1));
}


void LoginModule::user_login(const TcpMsgPtr& msg)
{
	//如果APP没有查找到账号文件, 而且没有获取到设备号,
	//那么登录时会返回一个objectid(时间戳+ip+进程号+随机序号)作为账号,
	//APP会将这个账号保存在隐藏文件
	//用户登录,APP获取隐藏文件读取账号信息,或者获取设备信息,
	//登录的时候将账号传过来,如果查询到数据库有,那么是老玩家,否者注册账号
	auto tnow = std::chrono::steady_clock::now();
	auto tnow_sec = tnow.time_since_epoch().count();
	const LoginRequest& login_req = msg->pbmessage<LoginRequest>();
	AccountInfo account_info;
	account_info.m_account = login_req.account();
	emLoginDeal login_deal = emLoginDeal::LOGIN_ACCOUNT;

	//是否需要生成账号
	if (is_need_generate_new_account(account_info.m_account, login_req.app_type()))
	{
		account_info.m_account = generate_account();
		login_deal = emLoginDeal::REGISTER_ACCOUNT;
	}

	if (account_info.m_account.empty())
	{
		msg->send_failed_reason(CODE_ACOCUNT_EMPTY);
		return;
	}

	//查询玩家账号失败, 需要注册账号
	if (!query_user_account(account_info))
	{
		login_deal = emLoginDeal::REGISTER_ACCOUNT;
	}
	UserInfo user_info;

	//如果是注册账号, 那么创建账号
	if (login_deal == emLoginDeal::REGISTER_ACCOUNT)
	{
		//生成一个新的uid
		int new_id = 0;

		//生成一个uid
		if (!query_new_userid(new_id))
		{
			msg->send_failed_reason(CODE_SYSTEM_ERROR);
			return;
		}
		//生成一个新玩家, 玩家信息.
		account_info.m_user_id = new_id;

		//写入redis, 保存7天
		RedisInstance::get_mutable_instance().json_to_hash(account_info.to_json(),
			account_redis_key(account_info.m_account), ACCOUNT_EXPIRE_SEC);
		user_info = generate_new_user(login_req, account_info);
		if (!insert_new_user(user_info))
		{
			msg->send_failed_reason(CODE_ACOCUNT_EMPTY);
			return;
		}

	}
	PB_MSG(LoginResponse, response);

	//生成token
	Json jv{ {"user_id" , account_info.m_user_id}, {"user_type", login_req.user_type()},
		{ "app_type", login_req.app_type()}, {"version",login_req.version()}, {"os_version", login_req.os_version()},
		{"channel", login_req.channel()}, {"time", tnow_sec} };

	response->set_hall_host(m_hall_list[rand() % m_hall_list.size()]);
	response->set_user_id(account_info.m_user_id);
	response->set_token(AES_ecb128_encrypt(jv.dump()));
	response->set_cur_version(m_versoin_info.m_cur_version);
	response->set_update_desc(m_versoin_info.m_update_desc);
	if (m_versoin_info.m_cur_version == login_req.version())
	{
		response->set_update_type(static_cast<int>(emUpdateType::NO_NEED_UPDATE));
	}
	else 
	{
		if (m_versoin_info.m_force_version >= login_req.version())
			response->set_update_type(static_cast<int>(emUpdateType::FORCE_UPDATE));
		else
			response->set_update_type(static_cast<int>(emUpdateType::NEED_UPDATE));
	}
	msg->send_response(CODE_SUCCESS, response);
	notify_user_login(account_info, login_req);
}

void LoginModule::notify_user_login(const AccountInfo& account_info, const LoginRequest& login_req)
{

	FULL_MSG(MqTag::HeaderType, head, NotifyLoginRequest, pbreq, MqTag, sendmsg);
	head->set_cmdtype(CMD_NOTIFY_LOGIN);
	head->set_send_exchange(EXCHANGE_LOGIN_TOPIC);
	head->set_send_routkey(ROUTE_USER_LOGIN);
	pbreq->set_user_id(account_info.m_user_id);
	pbreq->set_account(account_info.m_account);
	pbreq->set_device_code(login_req.device_code());
	pbreq->set_user_name(login_req.user_name());
	pbreq->set_screen_h(login_req.screen_h());
	pbreq->set_screen_w(login_req.screen_w());
	pbreq->set_resolution_ratio(login_req.resolution_ratio());
	pbreq->set_version(login_req.version());
	pbreq->set_app_type(login_req.app_type());
	pbreq->set_network_mode(login_req.network_mode());
	pbreq->set_os_version(login_req.os_version());
	pbreq->set_channel(login_req.channel());
	pbreq->set_user_type (login_req.user_type());
	LanchProcessInstance::get_mutable_instance().get_mq().send_message(sendmsg);
}

UserInfo LoginModule::generate_new_user(const LoginRequest& login_req, const AccountInfo& account_info)
{
	UserInfo user_info;
	user_info.m_account = account_info.m_account;
	user_info.m_user_id = account_info.m_user_id;
	user_info.m_device_code = login_req.device_code();
	user_info.m_device_name = login_req.device_name();
	user_info.m_app_type = login_req.app_type();
	user_info.m_user_sex = DEFAULT_SEX;
	user_info.m_user_name = login_req.user_name();
	user_info.m_header_url = login_req.header_url();
	if (user_info.m_header_url.empty())
	{
		user_info.m_header_url = m_default_male_header_urls[rand() % m_default_male_header_urls.size()];
	}

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
		<< BDocument{ bsoncxx::from_json(user_info .to_json().dump()).view() } << MGFinalize, op_update);
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

	m_timer->expires_from_now(m_max_login_sec);
	m_timer->async_wait(MEMFUN_THIS_BIND1(on_timeout, PLACEHOLDER::_1));

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
