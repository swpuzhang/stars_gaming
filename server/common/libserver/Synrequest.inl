#ifndef SYNC_REQUEST_INL
#define SYNC_REQUEST_INL

#include "Synrequest.h"
#include "libtools/FunctionBindMacro.h"
#include "libmessage/Message.h"
#include "Session.h"
#include "ErrorCode.pb.h"

template<typename MsgTag>
MessagePtr<MsgTag> SynRequest<MsgTag>::wait_response(MessagePtr<MsgTag> msg, 
	const std::chrono::milliseconds& milliseconds, SessionPtr<MsgTag> session)
{
	m_send_sequenceid = msg->header().sequenceid();
	{
		std::unique_lock<std::mutex>  lock(m_mutex);
		m_promise.reset(new std::promise<MessagePtr<MsgTag>>);
	}
	
	std::future<MessagePtr<MsgTag>> fu = m_promise->get_future();
	session->get_io_loop().post(std::bind(&Session<MsgTag>::write_fun, session, msg));

	MessagePtr<MsgTag> response; 
	std::future_status status = fu.wait_for(milliseconds);
	if (status == std::future_status::ready)
	{
		response = fu.get();
	}
	else if (status == std::future_status::timeout)
	{
		response = std::make_shared<Message<MsgTag>>();
		response->set_header(std::make_shared<typename MsgTag::HeaderType>());
		response->header().set_response_result(ErrorCode::CODE_WAIT_RESP_TIMEOUT);
		response->header().set_sequenceid(msg->header().sequenceid());
	}
	else
	{
		response = std::make_shared<Message<MsgTag>>();
		response->set_header(std::make_shared<typename MsgTag::HeaderType>());
		response->header().set_response_result(ErrorCode::CODE_SYSTEM_ERROR);
		response->header().set_sequenceid(msg->header().sequenceid());
	}
	m_send_sequenceid = -1;
	{
		std::unique_lock<std::mutex>  lock(m_mutex);
		m_promise.reset();
	}
	return response;
}

template<typename MsgTag>
void SynRequest<MsgTag>::response_come(const MessagePtr<MsgTag>& response)
{
	if ( m_send_sequenceid != response->header().sequenceid())
	{
		return;
	}
	std::unique_lock<std::mutex>  lock(m_mutex);
	if (m_promise)
	{
		m_promise->set_value(response);
	}
}

#endif