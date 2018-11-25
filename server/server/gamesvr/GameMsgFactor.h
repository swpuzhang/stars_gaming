#pragma once
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

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace LobbyCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;

class GameMsgFactor : public MessageMaker
{
public:
	GameMsgFactor() : MessageMaker()
	{
		register_message(CMD_USER_TOKEN_LOGIN_GAMESVR, UserTokenLoginRequest(), UserTokenLoginResponse());
	}
};