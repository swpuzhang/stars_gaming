#ifndef TYPES_H
#define TYPES_H
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <memory>
#include "AsioTypes.h"
#include "google/protobuf/message.h"


typedef char TY_INT8;
typedef unsigned char TY_UINT8;
typedef char TY_BYTE;
typedef short TY_INT16;
typedef unsigned short TY_UINT16;
typedef int TY_INT32;
typedef unsigned int TY_UINT32;
typedef long long TY_INT64;
typedef unsigned long long TY_UINT64;
typedef float TY_FLOAT;

template<class MsgTag>
class Message;

template<class MsgTag>
class Session;

class IoLoop;

using IoLoopPtr = std::shared_ptr<IoLoop>;

template<typename MsgTag>
using MessagePtr =std::shared_ptr<Message<MsgTag> > ;

template<class MsgTag>
using SessionPtr = std::shared_ptr<Session<MsgTag> > ;

using  SokcetPtr = std::shared_ptr<TCP_SPACE::socket> ;

template<class MsgTag>
using SessionWPtr = std::weak_ptr<Session<MsgTag> > ;

using PbMessagePtr = std::shared_ptr<google::protobuf::Message> ;

using PbMessage = google::protobuf::Message;

#endif