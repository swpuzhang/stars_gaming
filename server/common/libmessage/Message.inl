#ifndef MESSAGE_INL
#define  MESSAGE_INL
#include <cstdlib>

#include "MsgType.pb.h"

#include "MessageBuffer.h"
#include "Message.h"
#include "libtools/BigEndLittleEnd.h"
#include "liblog/Log.h"


template<class MsgTag>
Message<MsgTag>::Message()
{
	
}

template<class MsgTag>
bool Message<MsgTag>::send_response(int fail_reson, const PbMessagePtr& response) const
{
	TRACE_FUNCATION();
	EMPTY_MSG(MsgTag, msg);
	HEADER(HeaderType, head);
	head->set_cmdtype(header().cmdtype());
	head->set_type((header().type() ^ MsgType::REQUEST_MASK) | MsgType::RESPONSE_MASK);
	msg->set_header(head);
	msg->set_pbmessage(response);
	return m_session->send_response(msg);
}

template<class MsgTag>
bool Message<MsgTag>::send_response(const MessagePtr<MsgTag>& response) const
{
	TRACE_FUNCATION();
	response->header().set_cmdtype(header().cmdtype());
	response->header().set_type((header().type() ^ MsgType::REQUEST_MASK) | MsgType::RESPONSE_MASK);
	response->header().set_sequenceid(header().sequenceid());
	return m_session->send_response(response);
}


template<class MsgTag>
bool Message<MsgTag>::send_failed_reason(const int fail_reson) const
{
	
	MessagePtr<MsgTag> response = m_session->make_response(header().cmdtype());
	response->header().set_response_result(fail_reson);
	send_response(response);
	return false;
}

#endif