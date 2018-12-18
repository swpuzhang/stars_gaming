#ifndef MESAGE_TYPE_H

#define MESAGE_TYPE_H

#include <memory>
#include "MsgHeader.pb.h"


constexpr int  MSG_BUF_SIZE = 1024 * 128;
constexpr int  MQ_BUF_SIZE = 1024 * 1024 * 10;
constexpr int HEADER_LENGTH = sizeof(TY_UINT32);

class TcpTag 
{
public:
	typedef typename MsgHeader::Header HeaderType;
	typedef std::shared_ptr<typename MsgHeader::Header> HeaderPtr;
	static int get_max_buf_size()
	{
		return MSG_BUF_SIZE;
	}
};

class MqTag
{
public:
	typedef typename MsgHeader::MqHeader HeaderType;
	typedef std::shared_ptr<typename MsgHeader::MqHeader> HeaderPtr;
	static int get_max_buf_size()
	{
		return MQ_BUF_SIZE;
	}
};


/*template <class MsgTag>
class MsgTraits
{
	using HeaderType = typename MsgTag::HeaderType;
	using SessionType = MsgTag::SessionType;
	using MsgType = MsgTag::MsgType;

	
	using SessionPtr<MsgTag> = MsgTag::SessionPtr<MsgTag>;
	using MsgPtr = MsgTag::MsgPtr;
};*/

#endif // MESAGE_TYPE_H