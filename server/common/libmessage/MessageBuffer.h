#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H
#include <boost/shared_ptr.hpp>
#include <type_traits>

#include "Message.h"
#include "MessageType.h"

template <class MsgTag, class BufferType>
class MsgBuffer
{
public:
	int buffer_length() const { return HEADER_LENGTH + m_header_length + m_msg_length; }
	int header_length() const { return m_header_length; }
	int body_length() const { return m_msg_length; }
	BufferType buffer() const { return m_buffer; };
	BufferType body_buffer() const { return m_buffer + HEADER_LENGTH + m_header_length; }
	BufferType header_buffer() const { return m_buffer + HEADER_LENGTH; }

	bool decode_message(PbMessage& msg);
	void decode_header_length();
	bool decode_header(typename MsgTag::HeaderType& header);

protected:
	TY_UINT32 m_header_length;
	TY_UINT32 m_msg_length;
	BufferType m_buffer;
};

template<typename MsgTag>
class MsgConstBuffer : public MsgBuffer<MsgTag, const char*>
{
public:
	MsgConstBuffer(const char* buffer, TY_UINT32 msg_len) 
	{
		this->m_header_length = (0);
		this->m_msg_length = (msg_len);
		this->m_buffer = (buffer);
	}

	MsgConstBuffer() = delete;
	~MsgConstBuffer() {}
};

template <class MsgTag>
class MsgMutableBuffer : public MsgBuffer<MsgTag, char*>
{
public:
	MsgMutableBuffer();
	~MsgMutableBuffer();
	void encode_header_length(TY_UINT32 length);
	bool encode_header(const typename MsgTag::HeaderType& header);
	bool encode_message(const PbMessage& msg);
};

#include "MessageBuffer.inl"

#endif // !MESSAGE_H
