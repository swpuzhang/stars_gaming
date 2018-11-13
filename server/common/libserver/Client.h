#ifndef CLIENT_H
#define CLIENT_H

#include "libmessage/MessageType.h"
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "SessionManager.h"
#include "TcpSession.h"
#include <boost/weak_ptr.hpp>
#include <boost/atomic.hpp>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <fstream>
#include <memory>
#include <functional>
#include <boost/asio.hpp>
#include <memory>
#include <chrono>

class SendQueue;

class TcpSession;

class IoLoop;

class Client: public SessionManager
{
public:
	using QUEUE_PTR = std::shared_ptr<SendQueue>;
	using SELF_TYPE = Client;
	Client(IoLoop& ioloop, const std::shared_ptr<MessageDealer<TcpTag>>& msg_dealer,
		const std::shared_ptr<MessageMaker>& msg_maker);

	void close_session(const std::shared_ptr<TcpSession>& sessoin) override;

	void close_client();
	void connect(const std::string& ip, const short port_num);
	bool send_message(const std::shared_ptr<Message<TcpTag>>& msg);

	//发送和接收不能在同一线程
	bool send_message(const std::shared_ptr<Message<TcpTag>>& msg,
		std::shared_ptr<Message<TcpTag>>& response,
		const std::chrono::milliseconds &millisenconds = std::chrono::milliseconds(10000));

	bool send_message(const std::shared_ptr<Message<TcpTag>>& msg, 
		const ASYNC_FUN<TcpTag>& fun,
		const std::chrono::milliseconds &millisenconds = std::chrono::milliseconds(10000), IoLoop* io_loop = NULL);

	//void on_socket_close(Session* sock);

	std::shared_ptr<TcpSession> session_ptr();

private:

	void do_close_client();
	void do_connect(const std::string ip, const short port_num);
	void connect();
	void on_connet_complete(const SYSTEM_CODE& err_code);
	void on_wait_reconnect(const SYSTEM_CODE& err);
	void on_connect_timeout(const SYSTEM_CODE& err);
	void on_send_heart_beat(const SYSTEM_CODE& err);
	void register_connect_timer();
	void register_heart_beat_timer();
	void register_alive_timer();
	void on_check_alive(const SYSTEM_CODE& err);;

	std::weak_ptr<TcpSession> m_session_ref; 
	SokcetPtr m_socket;
	IoLoop& m_io_loop;
	
	TCP_SPACE::endpoint m_endpoint;
	boost::asio::deadline_timer m_reconnet_timer;
	boost::asio::deadline_timer m_connect_timer;
	boost::asio::deadline_timer m_heart_beat_timer;
	boost::asio::deadline_timer m_check_alive_timer;
	std::shared_ptr<MessageDealer<TcpTag>> m_message_dealer;
	std::shared_ptr<MessageMaker> m_msg_maker;
	int m_reconnect_interval;
	static int m_heart_beat_interval;
	static int check_link_alive_interval;
	static int m_alive_interval;
	bool m_auto_reconnect;
	bool m_is_connecting;
	bool m_is_wait_connect;
	bool m_is_connected;
};

#endif
