#ifndef LOGIN_MODULE_MANAGER_H
#define LOGIN_MODULE_MANAGER_H

#include "libtask_manager/TaskManager.h"
#include "libtools/IoLoop.h"
#include "libtools/Types.h"
#include "libtools/AsioTypes.h"
#include "libresource/Redis.h"
#include "libtools/JsonParser.h"
#include "UserCommon.h"
#include <boost/serialization/singleton.hpp>
#include <vector>
#include <map>
#include <chrono>

#include "ErrorCode.pb.h"
#include "LobbyMsg.pb.h"
#include "HallCmd.pb.h"
#include "SvrCmd.pb.h"
#include "SvrMsg.pb.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"
#include "libserver/TcpSession.h"
#include "LoginMsg.pb.h"
#include "LoginCmd.pb.h"

using namespace SystemMsg;
using namespace SystemCmd;

using namespace SvrMsg;
using namespace SvrCmd;
using namespace HallCmd;
using namespace LobbyMsg;
using namespace GameMsg;
using namespace GameCmd;
using namespace ErrorCode;
using namespace LoginCmd;
using namespace LoginMsg;

using namespace std::chrono;

class LobbyModule
{
public:
	using SELF_TYPE = LobbyModule;
	LobbyModule() {}
	void dispatch_app_msg(const TcpMsgPtr &msg);
private:


public:
	
private:
	
};

using LobbyInstance = boost::serialization::singleton<LobbyModule>;

#endif // !LOGIN_TASK_MANAGER_H