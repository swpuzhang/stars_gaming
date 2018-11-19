#pragma once

#include "Session.h"
#include "MqInfo.h"

#include "amqpcpp.h"
#include "amqpcpp/linux_tcp.h"
#include "libtools/Types.h"
#include "libmessage/Message.h"
#include "libmessage/MessageType.h"
#include "libserver/MqInfo.h"



class MqHandler;
class MessageMaker;
class MqMessageDealer;

class MqSession : public Session<MqTag>
{
public:
	
	using SELF_TYPE = MqSession;
	using SUPER_TYPE = Session<MqTag>;
	using MqSessionPtr = std::shared_ptr<MqSession>;

	MqSession(IoLoop& io_loop,
		const MqInfo &mq_config,
		const std::shared_ptr<MessageDealer<MqTag>>& msg_dealer,
		const std::shared_ptr<MessageMaker>& msg_maker);
	~MqSession();
	void connect();
	void do_write(MqMsgPtr msg) override;
	bool is_connected();
	void bind_one_exchange(const std::string& exchange_name, int exchange_type, const std::string& rout_key);
private:
	void init();
	void binding_exchange();
	void queue_success_call(const std::string &name, uint32_t messagecount,
		uint32_t consumercount);
	void queue_error_call(const char *message);
	bool recv_data(const AMQP::Message& message);
	void do_close();

private:
	std::unique_ptr<AMQP::TcpConnection> m_connection;
	std::unique_ptr<MqHandler> m_mq_handler;
	std::unique_ptr<AMQP::TcpChannel> m_channel;
	MqInfo m_mq_info;
	const char* m_recv_buf;
	int m_recv_len;
	bool m_is_session_opend;
	MsgMutableBuffer<MqTag> m_send_msg_buf;
};