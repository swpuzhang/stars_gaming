#ifndef MESSAGE_H
#define  MESSAGE_H
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "MsgHeader.pb.h"
#include "MessageType.h"

#include "MessageType.h"

#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "libtools/Types.h"


#define  HEADER(HeaderType, name)\
  auto name = std::make_shared<HeaderType>()
#define  PB_MSG(pb_type,name) \
  auto name = std::make_shared<pb_type>()
#define  EMPTY_MSG(MsgTag, name)\
  auto name = std::make_shared<Message<MsgTag>>()

#define  FULL_MSG(HeaderType, headname, PbType, pbname, MsgTag, name)\
  auto headname = std::make_shared<HeaderType>();\
  auto pbname = std::make_shared<PbType>();\
  auto name = std::make_shared<Message<MsgTag>>();\
  name->set_pbmessage(pbname);\
  name->set_header(headname)
  

using TcpMsg = Message<TcpTag>;

using MqMsg = Message<MqTag>;


using TcpMsgPtr = std::shared_ptr<TcpMsg>;

using MqMsgPtr = std::shared_ptr<MqMsg>;

class MqSession;
class TcpSession;

using TcpSessionPtr = std::shared_ptr<TcpSession>;
using MqSessionPtr = std::shared_ptr<MqSession>;

constexpr int  MSG_BUF_SIZE = 1024 * 128;
constexpr int  MQ_BUF_SIZE = 1024 * 1024 * 10;
constexpr int HEADER_LENGTH = sizeof(TY_UINT32);


template <class MsgTag>
class Message
{
public:
	
	enum
	{
		REQUEST_MASK = 0x4000,
		RESPONSE_MASK = 0x8000,
		NOFIFY_MASK = 0x0005,
		SYN_REQUEST = 0x4001,
		SYN_RESPONSE = 0x8001,
		ASYN_REQUEST = 0x4002,
		ASYN_RESPONSE = 0x8002,
		REGISTER_REQUEST = 0x4003,
		REGISTER_RESPONSE = 0x8003,
		HEARTBEAT_REQUEST = 0x4004,
		HEARTBEAT_RESPONSE = 0x8004,
		NOTIFY_REQUEST = 0x4005,
		NOTIFY_RESPONSE = 0x8005
	};
	using HeaderType = typename MsgTag::HeaderType;
	using HeaderPtr = typename MsgTag::HeaderPtr;
	Message();
	bool send_response(MessagePtr<MsgTag>& response) const;
	bool send_response(int fail_reson, PbMessagePtr& response) const;
	bool send_failed_reason(const int fail_reson) const;
	void set_session(const SessionPtr<MsgTag>& session) { m_session = session; }
	SessionPtr<MsgTag>& session_ptr()  { return m_session; }
	const SessionPtr<MsgTag>& session_ptr() const { return m_session; }
	const PbMessage& pbmessage() const { return *m_message; }
	PbMessage& pbmessage() { return *m_message; }
	template<typename T>
	T& pbmessage() const { return dynamic_cast<T&>(*m_message); }
	void set_pbmessage(const PbMessagePtr& msg) { m_message = msg; }
	void set_header(const HeaderPtr& header) { m_header = header; }
	HeaderType& header() { return *m_header; }
	HeaderType& header() const { return *m_header; }
	
private:

	HeaderPtr m_header;
	PbMessagePtr m_message;
	SessionPtr<MsgTag> m_session;
};

#include "Message.inl"

#endif