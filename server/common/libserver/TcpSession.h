#ifndef TCP_SESSION_H
#define TCP_SESSION_H
#define BOOST_THREAD_PROVIDES_FUTURE
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "libmessage/MessageBuffer.h"
#include "libmessage/Message.h"
#include "libmessage/MessageType.h"
#include "Synrequest.h"
#include "AsyncRequest.h"
#include "Session.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread/future.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <chrono>
#include <atomic>

class SendQueue;

class MessageMaker;
class SessionManager;
class IoLoop;

class TcpSession :  public Session<TcpTag>
{
public:
	using QUEUE_PTR = std::shared_ptr<SendQueue>;
	using SELF_TYPE = TcpSession;
	using SUPER_TYPE = Session<TcpTag>;
	using TcpSessionPtr = std::shared_ptr<TcpSession>;

	using TcpAsyncFun = ASYNC_FUN<TcpTag>;

	//TcpSession(boost::asio::io_context& service);
	TcpSession(IoLoop& io_loop, SokcetPtr new_socket,
		const std::shared_ptr<MessageDealer<TcpTag>>& msg_dealer,
		const std::shared_ptr<MessageMaker>& msg_maker,
		SessionManager* session_mana);
	~TcpSession() override;
	TCP_SPACE::socket& get_socket() { return m_socket; }
	void start();
	void check_alive();
	void set_sesssion_id(const int session_id) { m_session_id = session_id; }
	int get_session_id() {	return  m_session_id;}
	void set_alive_interval(const int seconds);
	void send_message(int cmd_type, const PbMessagePtr& msg);
	//发送和接收不能在同一线程
	void send_message(int cmd_type, const PbMessagePtr& msg, MessagePtr<TcpTag>& response,
		const std::chrono::milliseconds &millisenconds);
	void send_message(int cmd_type, const PbMessagePtr& msg, const ASYNC_FUN<TcpTag>& fun,
		const std::chrono::milliseconds &millisenconds,
		IoLoop* io_loop = nullptr);
	std::string get_addr() { return m_addr; }
	int get_heartbeat_interval() override { return m_alive_interval; }
	void do_write(MessagePtr<TcpTag> msg) override;
private:

	bool is_alive();
	void flush_alive();
	void do_close() override;
	void on_write_complete(const SYSTEM_CODE& err_code);
	void on_read_head_len_complete(const SYSTEM_CODE& err_code);
	void on_read_head_complete(const SYSTEM_CODE& err_code);
	void on_read_complete(const SYSTEM_CODE& err_code);
	void do_read_head_len();
	void do_read_head();
	void do_read_body();
	
	void set_addr();
	bool decode_header();
	bool make_msg();
	bool encode(Message<TcpTag>& msg);
	void do_check_alive();


	
	Message<TcpTag> m_cur_msg;
	SokcetPtr m_socket_ptr;
	TCP_SPACE::socket &m_socket;
	MsgMutableBuffer<TcpTag> m_recv_msg_buf;
	MsgMutableBuffer<TcpTag> m_send_msg_buf;
	QUEUE_PTR m_send_queue; 
	int m_alive_time;
	int m_session_id;
	int m_alive_interval;
	std::string m_addr;
	SessionManager* m_session_manager;
};

#endif