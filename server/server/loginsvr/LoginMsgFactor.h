#ifndef LOGIN_MSG_FACTOR_H
#define LOGIN_MSG_FACTOR_H


#include "libmessage/MessageMaker.h"
#include "libmessage/Message.h"


#include "LoginMsg.pb.h"
#include "LoginCmd.pb.h"

using namespace LoginCmd;
using namespace LoginMsg;

class LoginMsgFactor : public MessageMaker
{
public:
	LoginMsgFactor() : MessageMaker()
	{
		register_message(CMD_LOGIN, LoginRequest(), LoginResponse());
	}
};

#endif //LOGIN_MSG_FACTOR_H