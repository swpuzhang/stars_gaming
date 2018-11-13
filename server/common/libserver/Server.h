#ifndef SERVER_H
#define SERVER_H

#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "libmessage/MessageType.h"
#include <vector>
#include <memory>
#include <thread>
#include <boost/asio.hpp>

class TcpSession;
class MessageMaker;
class IoLoopPool;
class IoLoop;

template <typename MsgTag>
class MessageDealer;

class ServerSessionManager;

class Server
{
public:
	
	Server(IoLoop& io_loop, const std::shared_ptr<MessageDealer<TcpTag>>& msg_dealer, 
		const std::shared_ptr<MessageMaker>& msg_maker, 
		int thread_num = std::thread::hardware_concurrency(), 
		int max_sessoin = 5000);
	void start(const int port_num);

private:
	void accept();
	void on_accept_complete(const SYSTEM_CODE &err_code, 
		std::shared_ptr<TcpSession> new_session);
	IoLoop& m_acc_loop;
	std::shared_ptr<IoLoopPool> m_loop_pool;
	std::shared_ptr<TCP_SPACE::acceptor> m_accptor;
	std::shared_ptr<MessageDealer<TcpTag>> m_message_dealer;
	std::shared_ptr<MessageMaker> m_msg_maker;
	std::shared_ptr<ServerSessionManager> m_session_manager;
	//std::shared_ptr<ServerSessionManager> m_session_manager;
};

#endif

