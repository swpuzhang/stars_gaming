
#include "MsgHeader.pb.h"
#include "MqSession.h"
#include "MqHandler.h"
#include "libtools/AsioTypes.h"
#include "libtools/IoLoopPool.h"
#include "liblog/Log.h"
#include "libtools/FunctionBindMacro.h"
#include "libmessage/MessageDealer.h"
#include "libmessage/MessageBuffer.h"
#include "libmessage/MessageMaker.h"

MqSession::MqSession(IoLoop& io_loop,
	const MqInfo &mq_config,
	const std::shared_ptr<MessageDealer<MqTag>>& msg_dealer,
	const std::shared_ptr<MessageMaker>& msg_maker) :
	Session<MqTag>(io_loop, msg_dealer, msg_maker),
	m_connection(nullptr),
	m_mq_info(mq_config),
	m_recv_buf(nullptr), 
	m_recv_len(0),
	m_is_session_opend(false)
{
	
}

MqSession::~MqSession()
{
	
}


void MqSession::connect()
{
	m_io_loop.post(MEMFUN_CAST_BIND(init));
}

void MqSession::init()
{
	m_mq_handler = std::make_unique<MqHandler>(this->get_io_loop());
	m_connection = std::unique_ptr<AMQP::TcpConnection>(new  AMQP::TcpConnection(m_mq_handler.get(), 
		AMQP::Address(m_mq_info.m_ip, m_mq_info.m_port,
		m_mq_info.m_login, m_mq_info.m_work_dir)));
	m_channel = std::unique_ptr<AMQP::TcpChannel>(new AMQP::TcpChannel(m_connection.get()));
	auto shthis = this->shared_from_this();
	AMQP::SuccessCallback channel_ready_call = [this, shthis]() {
		INFO_LOG << ("channel ready");
		m_channel->declareQueue(m_mq_info.m_queue)
			.onSuccess(MEMFUN_CAST_HOLDER_BIND3(queue_success_call))
			.onError(MEMFUN_CAST_HOLDER_BIND1(queue_error_call));
		binding_exchange();
		m_is_session_opend = true;
		do_session_open();
	};

	AMQP::ErrorCallback channel_error_call = [this, shthis](const char *message) {
		ERROR_LOG << ("channel error");
	};

	m_channel->onReady(channel_ready_call);
	m_channel->onError(channel_error_call);
	
}

void MqSession::queue_success_call(const std::string &name, uint32_t messagecount,
	uint32_t consumercount)
{
	INFO_LOG << "declear queue(" << name << ") success messagecount(" << (messagecount)
		<< ") consumercount(" <<  (consumercount) + ")";
	binding_exchange();
	m_channel->consume(m_mq_info.m_queue).onMessage([this](const AMQP::Message &message, 
		uint64_t deliveryTag, bool redelivered) {
		m_channel->ack(deliveryTag);
		DEBUG_LOG << "mq recv data:" << (message.bodySize());
		recv_data(message);
	});
}

void MqSession::queue_error_call(const char *message)
{
	ERROR_LOG << "declear queue(" << m_mq_info.m_queue << ") error ";
}

void MqSession::binding_exchange()
{
	for (auto &one_exchange : m_mq_info.m_exchanges)
	{
		auto shthis = this->shared_from_this();
		AMQP::ErrorCallback exchange_error_call = [shthis, one_exchange](const char *message) {
			ERROR_LOG << "declare exChange(" << one_exchange.first << ") error ";
		};

		AMQP::SuccessCallback exchange_success_call = [this, shthis, one_exchange]() {
			INFO_LOG << "declare exChange(" << one_exchange.first << ") success ";
			auto iter_bindings = m_mq_info.m_binds.find(one_exchange.first);
			if (iter_bindings == m_mq_info.m_binds.end())
			{
				return;
			}
			for (const auto &e : iter_bindings->second)
			{
				m_channel->bindQueue(one_exchange.first, m_mq_info.m_queue, e)
					.onSuccess([=]()
				{
					INFO_LOG << "bindQueue(" << m_mq_info.m_queue << ") mode(" << one_exchange.second 
						<< ") to exchange(" + one_exchange.first << ") sucess ";
				})
					.onError([=](const char *message)
				{
					ERROR_LOG << "bindQueue(" << m_mq_info.m_queue << ") mode(" << one_exchange.second
						<< ") to exChange(" << e << ") error ";
				});
			}
		};

		m_channel->declareExchange(one_exchange.first, (AMQP::ExchangeType)one_exchange.second)
			.onSuccess(exchange_success_call)
			.onError(exchange_error_call);
	}

}


bool MqSession::recv_data(const AMQP::Message& message)
{
	TRACE_FUNCATION();
	MsgConstBuffer<MqTag> msg_buf(message.body(), message.bodySize());
	auto header = std::make_shared<MsgHeader::MqHeader>();
	msg_buf.decode_header_length();
	if (!msg_buf.decode_header(*header))
	{
		return false;
	}
	PbMessagePtr pbmsg = m_msg_maker->make_message(
		header->type(), header->cmdtype());

	if (!pbmsg)
	{
		WARN_LOG << "make msg cmd:" << header->cmdtype() << " failed";
		return false;
	}
	if (!msg_buf.decode_message(*pbmsg))
	{
		WARN_LOG << "make msg cmd:" << header->cmdtype() << " failed";
		return false;
	}
	EMPTY_MSG(MqTag, cur_msg);
	cur_msg->set_header(header);
	cur_msg->set_pbmessage(pbmsg);
	cur_msg->set_session(shared_from_this());
	DEBUG_LOG << "recv cmd:" << cur_msg->header().cmdtype() << " type:" 
		<< cur_msg->header().type() << " session:" << this;

	return true;
}

void MqSession::do_write(MqMsgPtr msg)
{
	auto &header = msg->header();
	auto &pbmsg = msg->pbmessage();
	if (header.ByteSize() + pbmsg.ByteSize() > MQ_BUF_SIZE)
	{
		ERROR_LOG << "mq_session(" << this << ") send msg(" << header.sequenceid()
			<< ") cmdtype (" << header.cmdtype() << ") type (" << header.type()
			<< ") msg size(" << header.ByteSize() + pbmsg.ByteSize() << ") over (" 
			<< MQ_BUF_SIZE << ")";
		return;
	}
	m_send_msg_buf.encode_header_length(header.ByteSize());
	if (!m_send_msg_buf.encode_header(header))
	{
		ERROR_LOG << "mq_session(" << this << ") send msg(" << header.sequenceid()
			<< ") cmdtype (" << header.cmdtype() << ") type (" << header.type()
			<< ") encode_header error";
		return;
	}
	if (m_send_msg_buf.encode_message(pbmsg))
	{
		ERROR_LOG << "mq_session(" << this << ") send msg(" << header.sequenceid()
			<< ") cmdtype (" << header.cmdtype() << ") type (" << header.type()
			<< ") encode_message error";
		return;
	}
	if (m_channel)
	{
		m_channel->publish(header.recv_exchange(), header.recv_routkey(), 
			m_send_msg_buf.buffer(), m_send_msg_buf.buffer_length());
	}
}

void MqSession::do_close()
{
	if (!m_is_session_opend)
	{
		return;
	}
	m_is_session_opend = false;
	m_mq_handler.reset();
	m_connection->close();
	FULL_MSG(MqTag::HeaderType, head, SystemMsg::SessionCloseRequest, 
		do_closereq, MqTag, msg);
	head->set_cmdtype(SystemCmd::CMD_SESSION_CLOSE);
	head->set_type(MsgType::NOTIFY_REQUEST);
	msg->set_session(std::dynamic_pointer_cast<MqSession>(shared_from_this()));
	//回调到上层
	m_recv_message_dealer->do_message_deal(msg);
	INFO_LOG << "session(" << this << ") do_close";

}

bool MqSession::is_connected()
{
	if (!m_mq_handler)
	{
		return false;
	}
	return m_mq_handler->is_connected();
}