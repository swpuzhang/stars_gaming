#include "GG_SiteMsg_Factory.h"
#include "GG_GameMsg.pb.h"
#include "GG_GameCmd.pb.h"
#include "GG_ErrorCode.pb.h"
#include "GG_RoomMsg.pb.h"
#include "GG_RoomCmd.pb.h"
#include "GG_SvrCmd.pb.h"
#include "GG_SvrMsg.pb.h"

using namespace GG_SvrMsg;
using namespace GG_SvrCmd;
using namespace GG_GameMsg;
using namespace GG_GameCmd;
using namespace GG_ErrorCode;
using namespace GG_RoomCmd;
using namespace GG_RoomMsg;

GG_SiteMsg_Factory::GG_SiteMsg_Factory(void)
{
}


GG_SiteMsg_Factory::~GG_SiteMsg_Factory(void)
{
}
void GG_SiteMsg_Factory::register_cmd_message()
{
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GET_GAMESVR_INSTANCE, GG_GetGameSvrInstanceRequest, GG_GetGameSvrInstanceResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_COMMON_QUERY, GG_CommQueryRequest, GG_CommQueryResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_SUBMIT_GAME_RESULT, GG_SubmitGameResultRequest, GG_SubmitGameResultResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_ADD_PLAYER_CHIPS, GG_AddPlayerChipsRequest, GG_SubmitGameResultResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_JOIN_GAME, GG_UserJoinGameRequest, GG_UserJoinGameResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_LEAVE_GAME, GG_UserLeaveGameRequest, GG_UserLeaveGameResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_ENTER_ROOM, GG_UserEnterRoomRequest, GG_UserEnterRoomResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_QUIT_ROOM, GG_UserQuitRoomRequest, GG_UserQuitRoomResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GAMESVR_START, GG_GameSvrStartRequest, GG_GameSvrStartResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_OPEN_GAME_ROOM, GG_OpenGameRoomRequest, GG_OpenGameRoomResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GAME_ROOM_EMPTY, GG_GameRoomEmptyRequest, GG_GameRoomEmptyResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GAMESVR_ALIVE, GG_GameSvrAliveRequest, GG_GameSvrAliveResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_FLUSH_USER_IN_GAME, GG_FlushUserInGameRequest, GG_FlushUserInGameResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GET_INGAME_COUNT, GG_GetInGameCountRequest, GG_GetInGameCountResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_CHECK_USER_LOGIN, GG_CheckUserLoginRequest, GG_CheckUserLoginResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_KICK_OUT_USER, GG_KickOutUserRequest, GG_CheckUserLoginResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_FLUSH_USER_GAMESESSION, GG_FlushUserGameSessionRequest, GG_FlushUserGameSessionResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_TELL_USER_LOGOUT_GAME, GG_TellUserLogoutGameRequest, GG_TellUserLogoutGameResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GAME_BUY_CHIPS_IN, GG_GameBuyChipsInRequest, GG_GameBuyChipsInResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GAME_OVER_ACCOUNT, GG_GameOverAccountRequest, GG_GameOverAccountResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_GAME_LEAVE_ACCOUNT, GG_GameLeaveAccountRequest, GG_GameLeaveAccountResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_BOX_TIME_PAUSE, GG_UserBoxTimePauseRequest, GG_UserBoxTimePauseResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_BOX_TIME_RECOVER, GG_UserBoxTimeRecoverRequest, GG_UserBoxTimeRecoverResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_GAME_START_BET, GG_GameStartBetRequest, GG_GameStartBetResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_GAME_ADD_CARRY, GG_GameAddCarryRequest, GG_GameAddCarryResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_MATCH_OVER_ACCOUNT, GG_MatchOverAccountRequest, GG_MatchOverAccountResponse);

    REGISTER_GG_CMD_MESSAGE(GG_CMD_USERLOG_RECORD, GG_UserLogRecordRequest, GG_UserLogRecordResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_GAMELOG_RECORD, GG_GameLogRequest, GG_GameLogResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_APPLOG_RECORD, GG_AppLogRecordRequest, GG_AppLogRecordResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_CHIPS_LOG, GG_UserChipsLogRequest, GG_UserChipsLogResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_HALL_LOGIN_RECORD, GG_HallLoginRecordRequest, GG_HallLoginRecordResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_GAME_LOGIN_RECORD, GG_GameLoginRecordRequest, GG_GameLoginRecordResponse);
    REGISTER_GG_CMD_MESSAGE(GG_CMD_FLUSH_ROOM_ALIVE, GG_FlushRoomAliveRequest, GG_FlushRoomAliveResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_ESCAPE_GAME, GG_UserEscapeGameRequest, GG_UserEscapeGameResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_FLUSH_ROOM_IDLE, GG_FlushRoomIdleRequest, GG_FlushRoomIdleResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_USER_LOGIN_GAMESVR, GG_UserLoginRequest, GG_UserLoginResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_NOTIFY_OPEN, GG_NotifyOpenRequest, GG_NotifyOpenResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_CHECK_USER_TOKEN, GG_CheckUserTokenRequest, GG_CheckUserTokenResponse);

	REGISTER_GG_CMD_MESSAGE(GG_CMD_FLUSH_ROOM_STATUS, GG_FlushRoomStausRequest, GG_FlushRoomStausResponse);
	REGISTER_GG_CMD_MESSAGE(GG_CMD_NOTIFY_ROOM_STATUS, GG_NotifyRoomStausRequest, GG_NotifyRoomStausResponse);
	
}
