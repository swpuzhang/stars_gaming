#ifndef CLIENT_H
#define CLIENT_H
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/atomic.hpp>
#include <vector>
#include <string>
#include "../libtools/Types.h"
#include "../libtools/AsioTypes.h"
#include "../libmessage/MessageBuffer.h"
#include "../libmessage/BasicMessageDealer.h"

class CSendQueue;

class CClient : public boost::enable_shared_from_this<CClient>
{
public:
	typedef boost::shared_ptr<CSendQueue> QUEUE_PTR;
	typedef CClient SELF_TYPE;
	CClient(boost::asio::io_service& sevice, const std::string& ip, 
		const short port_num);
	
	void connect();
	bool send_message(MESSAGE_PTR msg);
	void register_messager_dealer(const boost::shared_ptr<CMessageDealer>& msg_dealer);
	~CClient();
private:
	void on_connet_complete(const SYSTEM_CODE& err_code);
	void on_wait_reconnect(const SYSTEM_CODE& err);
	void on_write_complete(const SYSTEM_CODE& err_code);
	void on_read_head_complete(const SYSTEM_CODE& err_code);
	void on_read_complete(const SYSTEM_CODE& err_code);
	void do_read_head();
	void do_read_body();
	void do_write(MESSAGE_PTR msg);
	void register_connect_timer();
	void handle_message();
	void on_connect_timeout(const SYSTEM_CODE& err);
	void register_heart_beat_timer();
	void register_alive_timer();
	void on_check_alive(const SYSTEM_CODE& err);
	void on_send_heart_beat(const SYSTEM_CODE& err);

	TCP_SPACE::socket m_socket;
	int m_reconnect_interval;
	QUEUE_PTR m_send_queue; 
	TCP_SPACE::endpoint m_endpoint;
	boost::asio::deadline_timer m_reconnet_timer;
	boost::atomic_bool m_is_connected;
	CMessageBuffer m_recv_msg_buf;
	CMessageBuffer m_send_msg_buf;
	boost::shared_ptr<CMessageDealer> m_message_dealer;
	boost::asio::deadline_timer m_connect_timer;
	static int m_heart_beat_interval;
	boost::asio::deadline_timer m_heart_beat_timer;
	static int check_link_alive_interval;
	boost::asio::deadline_timer m_check_alive_timer;
	static int m_alive_interval;
};

#endif
