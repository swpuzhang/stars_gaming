#ifndef MESSAGE_BUFFER_INL
#define MESSAGE_BUFFER_INL

#include "libtools/BigEndLittleEnd.h"
#include "MessageType.h"
#include "MessageMaker.h"
#include "MessageBuffer.h"
#include "liblog/Log.h"
#include "Message.h"
#include <cstdlib>


template <class MsgTag, class BufferType>
bool MsgBuffer<MsgTag, BufferType>::decode_message(PbMessage& msg)
{
	BufferType msg_buf = m_buffer + HEADER_LENGTH + m_header_length;
	if (!msg.ParseFromArray(msg_buf, m_msg_length))
	{
		ERROR_LOG << "decode header failed header_len:" << m_header_length;
		return false;
	}
	return true;
}

template <class MsgTag, class BufferType>
void MsgBuffer<MsgTag, BufferType>::decode_header_length()
{
	const TY_UINT32* buf = reinterpret_cast<const TY_UINT32*>(m_buffer);
	m_header_length = ntoh32(*buf);
}

template <class MsgTag, class BufferType>
bool MsgBuffer<MsgTag, BufferType>::decode_header(typename MsgTag::HeaderType& header)
{
	BufferType header_buf = m_buffer + HEADER_LENGTH;
	if (!header.ParseFromArray(header_buf, m_header_length))
	{
		ERROR_LOG << "decode header failed header_len:" << m_header_length;
		return false;
	}
	m_msg_length = header.length();
	return  true;
}

template <class MsgTag>
MsgMutableBuffer<MsgTag>::MsgMutableBuffer() 
{
	this->m_header_length = (0);
	this->m_msg_length = (0);
	this->m_buffer = (new TY_BYTE[MsgTag::get_max_buf_size() + HEADER_LENGTH]);
}

/*template <>
MsgMutableBuffer<MqTag>::MsgMutableBuffer() 
{
	this->m_header_length = (0);
	this->m_msg_length = (0);
	this->m_buffer = (new TY_BYTE[MQ_BUF_SIZE + HEADER_LENGTH]);
}*/

template <class MsgTag>
MsgMutableBuffer<MsgTag>::~MsgMutableBuffer()
{ 
	if (this->m_buffer)
	{
		delete[] this->m_buffer;
		this->m_buffer = NULL;
	}
}

template <class MsgTag>
void MsgMutableBuffer<MsgTag>::encode_header_length(TY_UINT32 length)
{
	this->m_header_length = length;
	TY_UINT32 nint = hton32(this->m_header_length);
	*(reinterpret_cast<TY_UINT32*>(this->m_buffer)) = nint;
}

template <class MsgTag>
bool MsgMutableBuffer<MsgTag>::encode_header(const typename MsgTag::HeaderType& header)
{
	TY_BYTE* header_buf = this->m_buffer + HEADER_LENGTH;
	this->m_msg_length = header.length();
	if (!header.SerializeToArray(header_buf, header.ByteSize()))
	{
		ERROR_LOG << "msg encode header error";
		return false;
	}

	return true;
}

template <class MsgTag>
bool MsgMutableBuffer<MsgTag>::encode_message(const PbMessage& msg)
{
	TY_BYTE* msg_buf = this->m_buffer + HEADER_LENGTH + this->m_header_length;
	if (!msg.SerializeToArray(msg_buf, msg.ByteSize()))
	{
		ERROR_LOG << "msg encode error";
		return false;
	}

	return true;
}




#endif // !MESSAGE_BUFFER_INL