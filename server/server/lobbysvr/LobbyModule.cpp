#include <functional>
#include <random>
#include <chrono>

#include "LobbyModule.h"
#include "LanchProcess.h"
#include "MqRoute.h"
#include "libtools/FunctionBindMacro.h"
#include "libtools/JsonParser.h"
#include "libtools/SystemTool.h"
#include "libresource/Mongodb.h"
#include "libresource/Redis.h"
#include "libtools/BaseTool.h"

#include "libmessage/Message.h"

void LobbyModule::dispatch_app_msg(const TcpMsgPtr &msg)
{

}

