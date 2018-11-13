#ifndef ASYNC_REQUEST_INL
#define ASYNC_REQUEST_INL

#include "AsyncRequest.h"
#include "libtools/IoLoopPool.h"
#include "Session.h"
#include "ErrorCode.pb.h"


template <class MsgTag>
AsynItem<MsgTag>::AsynItem() : m_sequnce_id(0) {}

template <class MsgTag>
AsynItem<MsgTag>::AsynItem(IoLoop* io_loop, 
	const ASYNC_FUN<MsgTag>& fun, 
	const SessionPtr<MsgTag>& session, 
	TY_UINT64 sequnce_id,
	const MessagePtr<MsgTag>& request, 
	const std::chrono::milliseconds& milliseconds,
	AsyncRequest<MsgTag>* manager) :

	m_sequnce_id(sequnce_id),
	m_io_loop(io_loop), 
	m_request(request),
	m_fun(fun),
	m_timer(std::make_shared<boost::asio::deadline_timer>
	(manager->get_io_loop())), m_manager(manager)
{
	//Æô¶¯¶¨Ê±Æ÷
	m_timer->expires_from_now(boost::posix_time::milliseconds(milliseconds.count()));

	//m_connec_timer.async_wait(MEMFUN_THIS_BIND2(on_connect_timeout, PLACEHOLDER::_1, new_session));
	m_timer->async_wait(std::bind(&AsyncRequest<MsgTag>::on_response_timeout, m_manager,
		std::placeholders::_1, m_sequnce_id, session));
}

template <class MsgTag>
void AsynItem<MsgTag>::on_response_come(const MessagePtr<MsgTag>& response)
{
	TRACE_FUNCATION();
	m_timer->cancel();
	if (!m_io_loop)
	{
		m_manager->get_io_loop().post(std::bind(m_fun, m_request, response));
	}
	else
	{
		m_io_loop->post(std::bind(m_fun, m_request, response));
	}

}

template <class MsgTag>
void AsynItem<MsgTag>::on_response_timeout(const MessagePtr<MsgTag>& response)
{
	TRACE_FUNCATION();
	if (!m_io_loop)
	{
		m_manager->get_io_loop().post(std::bind(m_fun, m_request, response));
	}
	else
	{
		m_io_loop->post(std::bind(m_fun, m_request, response));
	}
}

template <class MsgTag>
AsyncRequest<MsgTag>::AsyncRequest(IoLoop& io) : m_io(io) {}

template <class MsgTag>
void AsyncRequest<MsgTag>::on_response_come(const MessagePtr<MsgTag>& response)
{
	TRACE_FUNCATION();
	TY_UINT64 sequnceid  = response->header().sequenceid();
	auto it = m_items.find(response->header().sequenceid());
	if (it == m_items.end())
	{
		WARN_LOG << "sequnceid(" << sequnceid << ") have no request";
		return;
	}
	it->second.on_response_come(response);
}
 
template <class MsgTag>
void AsyncRequest<MsgTag>::on_response_timeout(const SYSTEM_CODE& err_code, 
	TY_UINT64 sequnceid, SessionPtr<MsgTag> session)
{
	TRACE_FUNCATION();
	auto it = m_items.find(sequnceid);
	assert(it != m_items.end());
	if (!err_code) 
	{
		MessagePtr<MsgTag> response = 
			session->make_response(it->second.m_request->header().cmdtype());
		response->header().set_response_result(ErrorCode::CODE_WAIT_RESP_TIMEOUT);
		response->header().set_sequenceid(sequnceid);
		it->second.on_response_timeout(response);
	}
	m_items.erase(it);
}

template <class MsgTag>
void AsyncRequest<MsgTag>::send_request(IoLoop* io_loop,
	const MessagePtr<MsgTag>& request,
	const ASYNC_FUN<MsgTag>& fun,
	const std::chrono::milliseconds& milliseconds,
	const SessionPtr<MsgTag>& session)
{
	TRACE_FUNCATION();
	auto it = m_items.find(request->header().sequenceid());
	if (it != m_items.end())
	{
		return;
	}

	m_items.insert({ request->header().sequenceid(),
		AsynItem<MsgTag>(io_loop, fun, session, request->header().sequenceid(), 
			request, milliseconds, this) });
}

#endif

