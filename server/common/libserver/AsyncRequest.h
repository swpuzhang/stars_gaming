#ifndef ASYNC_REQUEST_H
#define ASYNC_REQUEST_H

#include "libmessage/Message.h"
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "libtools/IoLoopPool.h"
#include "libtools/IoLoop.h"
#include "liblog/Log.h"
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/thread/future.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <chrono>
#include <map>

template <class MsgTag>
using ASYNC_FUN = std::function<void(const MessagePtr<MsgTag>& request, const MessagePtr<MsgTag>& response)>;

class ThreadPool;

template <class MsgTag>
class AsyncRequest;

template <class MsgTag>
class Session;

template <class MsgTag>
struct AsynItem
{
	AsynItem();
	AsynItem(IoLoop* io_loop, const ASYNC_FUN<MsgTag>& fun, 
		const SessionPtr<MsgTag>& session, TY_UINT64 sequnce_id,
		const MessagePtr<MsgTag>& request, 
		const std::chrono::milliseconds& milliseconds, 
		AsyncRequest<MsgTag>* manager);

	TY_UINT64 m_sequnce_id;
	IoLoop* m_io_loop;
	MessagePtr<MsgTag> m_request;
	ASYNC_FUN<MsgTag> m_fun;
	std::shared_ptr<boost::asio::deadline_timer> m_timer;
	AsyncRequest<MsgTag>* m_manager;

	void on_response_come(const MessagePtr<MsgTag>& response);

	void on_response_timeout(const MessagePtr<MsgTag>& response);
};

template <class MsgTag>
class AsyncRequest
{
public:
	AsyncRequest(IoLoop& io);

	IoLoop& get_io_loop() { return  m_io; }

	void on_response_come(const MessagePtr<MsgTag>& response);

	void on_response_timeout(const SYSTEM_CODE& err_code, 
		TY_UINT64 sequnceid, SessionPtr<MsgTag> session);

	void send_request(IoLoop* th_pool,
		const MessagePtr<MsgTag>& request,
		const ASYNC_FUN<MsgTag>& fun,
		const std::chrono::milliseconds& milliseconds,
		const SessionPtr<MsgTag>& session);

private:

	std::map<TY_UINT64, AsynItem<MsgTag>> m_items;
	IoLoop& m_io;
};

#include "AsyncRequest.inl"

#endif // !ASYNC_REQUEST_H