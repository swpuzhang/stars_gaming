#ifndef MESAGE_TYPE_H

#define MESAGE_TYPE_H

#include <memory>
#include "MsgHeader.pb.h"


class TcpTag 
{
public:
	typedef typename MsgHeader::Header HeaderType;
	typedef std::shared_ptr<typename MsgHeader::Header> HeaderPtr;
};

class MqTag
{
public:
	typedef typename MsgHeader::MqHeader HeaderType;
	typedef std::shared_ptr<typename MsgHeader::MqHeader> HeaderPtr;
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