#ifndef LOGIN_MSG_FACTOR_H
#define LOGIN_MSG_FACTOR_H


#include "libmessage/MessageMaker.h"
#include "libmessage/Message.h"
#include "ErrorCode.pb.h"
#include "HallMsg.pb.h"
#include "HallCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace HallCmd;
using namespace HallMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

class LoginMsgFactor : public MessageMaker
{
public:
	LoginMsgFactor() : MessageMaker()
	{
		register_message(CMD_USER_TOKEN_LOGIN_GAMESVR, UserTokenLoginRequest(), UserTokenLoginResponse());
	}
};

#endif //LOGIN_MSG_FACTOR_H