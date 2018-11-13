#ifndef SESSION_H
#define SESSION_H
#define BOOST_THREAD_PROVIDES_FUTURE
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "libmessage/MessageBuffer.h"
#include "libmessage/Message.h"
#include "Synrequest.h"
#include "AsyncRequest.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread/future.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <chrono>
#include <atomic>


class SendQueue;

template <typename MsgTag>
class MessageDealer;

class MessageMaker;

class IoLoop;

template <typename MsgTag>
class Session : public std::enable_shared_from_this<Session<MsgTag>>
{
public:
	using QUEUE_PTR = std::shared_ptr<SendQueue>;
	using SELF_TYPE = Session<MsgTag>;
	

	virtual ~Session();
	//Session(boost::asio::io_context& service);
	Session(IoLoop& io_loop,
		const std::shared_ptr<MessageDealer<MsgTag> >& msg_dealer,
		const std::shared_ptr<MessageMaker>& msg_maker);
	MessagePtr<MsgTag> make_response(const int cmd_type);

	void close();

	void send(const MessagePtr<MsgTag>& msg);
	//发送和接收不能在同一线程
	void send(const MessagePtr<MsgTag>& msg, MessagePtr<MsgTag>& response, 
		const std::chrono::milliseconds &millisenconds);

	void send(const MessagePtr<MsgTag>& msg, const ASYNC_FUN<MsgTag>& fun,
		const std::chrono::milliseconds &millisenconds,
		IoLoop* io_loop = nullptr);

	void response_come(const MessagePtr<MsgTag>& msg);

	void asyn_response_come(const MessagePtr<MsgTag>& msg);

	bool send_response(const MessagePtr<MsgTag>& msg);

	IoLoop& get_io_loop() { return  m_io_loop; }

	void write_fun(MessagePtr<MsgTag> msg);

	virtual int get_heartbeat_interval() { return 0; }
protected:

	void do_session_open();
	void set_msg_sequenceid(const MessagePtr<MsgTag>& msg);
	bool send_pre_deal(const MessagePtr<MsgTag>& msg);
	void do_send_message(MessagePtr<MsgTag> msg);
	void do_send_syn_message(MessagePtr<MsgTag> msg, MessagePtr<MsgTag>& response, 
		const std::chrono::milliseconds millisenconds);
	void do_send_asyn_message(MessagePtr<MsgTag> msg, const ASYNC_FUN<MsgTag> fun, 
		const std::chrono::milliseconds millisenconds,
		IoLoop* io_loop);

	virtual void do_write(MessagePtr<MsgTag> msg) = 0;
	void close_fun();
	virtual void do_close() = 0;

protected:
	IoLoop& m_io_loop;
	std::shared_ptr<MessageDealer<MsgTag>> m_recv_message_dealer;
	std::shared_ptr<MessageMaker> m_msg_maker;
	SynRequest<MsgTag> m_sync_request;
	AsyncRequest<MsgTag> m_async_request;
	std::atomic<TY_UINT64> m_sequenceid;
};

#include "Session.inl"
#endif