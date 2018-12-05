#ifndef LOGIN_MSG_FACTOR_H
#define LOGIN_MSG_FACTOR_H


#include "libmessage/MessageMaker.h"
#include "libmessage/Message.h"
#include "ErrorCode.pb.h"
#include "LobbyMsg.pb.h"
#include "LobbyCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"
#include "LoginCmd.pb.h"
#include "LoginMsg.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;
using namespace LoginMsg;
using namespace LoginCmd;

class LoginMsgFactor : public MessageMaker
{
public:
	LoginMsgFactor() : MessageMaker()
	{
		register_message(CMD_USER_TOKEN_LOGIN, UserTokenLoginRequest(), UserTokenLoginResponse());
	}
};

#endif //LOGIN_MSG_FACTOR_H