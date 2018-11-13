#include "GG_SecurityMsg_Factory.h"
//#include "GG_DCenter_Command.h"
//#include "GG_SvrInteract_Message.h"

GG_SecurityMsg_Factory::GG_SecurityMsg_Factory(void)
{
}


GG_SecurityMsg_Factory::~GG_SecurityMsg_Factory(void)
{
}


GG_Message* GG_SecurityMsg_Factory::make_request(int cmd_type)
{
	/*GG_MAKE_REQUEST_BEGIN(cmd_type);
	GG_MAKE_MESSAGE(GG_CMD_DISPATCH_USER_MSG, GG_DispatchUserMsg_Request);
	GG_MAKE_MESSAGE(GG_CMD_FLUSH_USER_ONLINE, GG_FlushUserOnline_Request);
	GG_MAKE_MESSAGE(GG_CMD_TELL_USER_OFFLINE, GG_TellUserOffline_Request);
	GG_MAKE_MESSAGE(GG_CMD_FLUSH_USER_IN_GAME, GG_FlushUserInGame_Request);
	GG_MAKE_MESSAGE(GG_CMD_CHECK_USER_TOKEN, GG_CheckUserToken_Request);
	GG_MAKE_MESSAGE(GG_CMD_KICK_OUT_USER, GG_KickOutUser_Request);
	GG_MAKE_MESSAGE(GG_CMD_FORBIDDEN_USER, GG_ForbiddenUser_Request);
	GG_MAKE_MESSAGE(GG_CMD_CHECK_USER_LOGIN, GG_CheckUserLogin_Request);
	GG_MAKE_REQUEST_END(cmd_type, GG_Message_Factory);*/
}

GG_Message* GG_SecurityMsg_Factory::make_response(int cmd_type)
{
	/*GG_MAKE_RESPONSE_BEGIN(cmd_type);
	GG_MAKE_MESSAGE(GG_CMD_DISPATCH_USER_MSG, GG_DispatchUserMsg_Response);
	GG_MAKE_MESSAGE(GG_CMD_FLUSH_USER_ONLINE, GG_FlushUserOnline_Response);
	GG_MAKE_MESSAGE(GG_CMD_TELL_USER_OFFLINE, GG_TellUserOffline_Response);
	GG_MAKE_MESSAGE(GG_CMD_FLUSH_USER_IN_GAME, GG_FlushUserInGame_Response);
	GG_MAKE_MESSAGE(GG_CMD_CHECK_USER_TOKEN, GG_CheckUserToken_Response);
	GG_MAKE_MESSAGE(GG_CMD_KICK_OUT_USER, GG_KickOutUser_Response);
	GG_MAKE_MESSAGE(GG_CMD_FORBIDDEN_USER, GG_ForbiddenUser_Response);
	GG_MAKE_MESSAGE(GG_CMD_CHECK_USER_LOGIN, GG_CheckUserLogin_Response);
	GG_MAKE_RESPONSE_END(cmd_type, GG_Message_Factory);*/
}

std::string GG_SecurityMsg_Factory::get_cmd_type_name(int cmd_type)
{
	/*GG_CMD_TYPE_NAME_BEGIN(cmd_type);
	GG_CMD_TYPE_NAME(GG_CMD_DISPATCH_USER_MSG);
	GG_CMD_TYPE_NAME(GG_CMD_FLUSH_USER_ONLINE);
	GG_CMD_TYPE_NAME(GG_CMD_TELL_USER_OFFLINE);
	GG_CMD_TYPE_NAME(GG_CMD_FLUSH_USER_IN_GAME);
	GG_CMD_TYPE_NAME(GG_CMD_CHECK_USER_TOKEN);
	GG_CMD_TYPE_NAME(GG_CMD_KICK_OUT_USER);
	GG_CMD_TYPE_NAME(GG_CMD_FORBIDDEN_USER);
	GG_CMD_TYPE_NAME(GG_CMD_CHECK_USER_LOGIN);
	GG_CMD_TYPE_NAME_END(cmd_type, GG_Message_Factory);*/
}

