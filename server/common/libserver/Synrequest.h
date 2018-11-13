#ifndef SYNC_REQUEST_H
#define SYNC_REQUEST_H

#define BOOST_THREAD_PROVIDES_FUTURE
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <thread>
#include <future>
#include <mutex>
#include <chrono>
#include <atomic>

template<typename MsgTag>
class SynRequest
{
public:
	SynRequest() : m_send_sequenceid(0){}
	MessagePtr<MsgTag> wait_response(MessagePtr<MsgTag> msg, 
		const std::chrono::milliseconds& milliseconds, SessionPtr<MsgTag> session);
	void response_come(const MessagePtr<MsgTag>& response);
private:
	std::shared_ptr<std::promise<MessagePtr<MsgTag>> > m_promise;
	std::mutex m_mutex;
	std::atomic<std::uint64_t> m_send_sequenceid;
};

#include "Synrequest.inl"
#endif