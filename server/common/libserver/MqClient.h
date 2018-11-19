#ifndef MQ_CLIENT_H
#define MQ_CLIENT_H
#include <boost/atomic.hpp>
#include <boost/asio.hpp>
#include <functional>
#include <vector>
#include <fstream>
#include <memory>
#include <chrono>

#include "MqSession.h"
#include "MqInfo.h"

#include "libmessage/Message.h"
#include "libmessage/MessageType.h"
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "libtools/Json"

class MqSession;

class IoLoop;

class MqClient
{
public:
	using SELF_TYPE = MqClient;
	MqClient(IoLoop& ioloop, const std::string &mq_config_str, const std::string mq_sub_name,
		const std::shared_ptr<MessageDealer<MqTag>>& msg_dealer,
		const std::shared_ptr<MessageMaker>& msg_maker);
	~MqClient();
	void close_client();
	void connect();
	bool send_message(const std::shared_ptr<Message<MqTag>>& msg);
	void bind_exchange(const std::string& exchange_name, int exchange_type, const std::string& rout_key);

	//发送和接收不能在同一线程
	bool send_message(const std::shared_ptr<Message<MqTag>>& msg, 
		std::shared_ptr<Message<MqTag>>& response,
		const std::chrono::milliseconds &millisenconds = std::chrono::milliseconds(10000));

	bool send_message(const std::shared_ptr<Message<MqTag>>& msg,
		const ASYNC_FUN<MqTag>& fun,
		const std::chrono::milliseconds &millisenconds = std::chrono::milliseconds(10000),
		IoLoop* io_loop = NULL);

	std::shared_ptr<MqSession>& session_ptr() {return m_session;}

private:
	void do_bind(const std::string& exchange_name, int exchange_type, const std::string& rout_key);
	void do_close_client();
	void do_connect();
	void connect_mq();
	void on_timeout(const SYSTEM_CODE& err);
	void register_timer();
	void close_session();
	IoLoop& m_io_loop;
	MqInfo m_mq_info;
	std::shared_ptr<MqSession> m_session;
	boost::asio::deadline_timer m_timer;
	std::shared_ptr<MessageDealer<MqTag>> m_message_dealer;
	std::shared_ptr<MessageMaker> m_msg_maker;
	int m_interval;
	bool m_auto_reconnect;
	bool m_is_connecting;
};

#endif
