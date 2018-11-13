#include "GG_GameArea_Manager.h"
#include "GG_GameArea.h"
#include "GG_Game_Process.h"
#include "ace/Select_Reactor.h"
#include "logger/ilog.h"
#include "GG_GameCmd.pb.h"
#include "GG_GameMsg.pb.h"
#include "json/json.h"
#include "GG_Card_Dealer.h"
#include "GG_GamePlayer_Manager.h"
#include "GG_Game_Process.h"
#include "libggppss/GG_Server.h"
#include <assert.h>
#include <stdint.h>
#include "libggppcc/GG_MQService.h"
#include "GG_GameDesk.h"
#include "ggpp/GG_SiteName.h"
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

#define GG_SQL_GAMESVR_LOAD_ROOM_ID "GG_SQL_GAMESVR_LOAD_ROOM_ID"

const int GAMESVR_ALIVE_INTERVAL = 60;

GG_GameArea_Manager::GG_GameArea_Manager(void)
	: m_svr_port(0), m_is_running(true), m_preactor(NULL), m_check_player_timer(-1), m_last_flush_time(0)
{
	
}


void GG_GameArea_Manager::parse_rid_uid_from_msg(const GG_Message &msg, int& userId, int& roomId)
{
	PARSE_MESSAGE_BEGIN(msg.m_cmd_type);
	PARSE_MESSAGE(GG_CMD_APPLY_ENTER_ROOM,GG_ApplyEnterRoomRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_SEAT, GG_ApplySeatRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_QUIT_ROOM, GG_ApplyQuitRoomRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_FLUSHROOM, GG_ApplyFlushRoomRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_FACESHOW, GG_ApplyFaceShowRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_CHAT, GG_ApplyChatRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_USE_PROPS, GG_ApplyUsePropsRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_PRESENT_GIFT, GG_ApplyPresentGiftRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_TRANSMIT_MESSAGE, GG_ApplyTransmitMessageRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_PRESENT_GIFT_NEWINTER, GG_ApplyPresentGiftNewInterRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_STANDUP, GG_ApplyStandupRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_POST_CARD, GG_ApplyPostCardRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_DIG_CARD, GG_ApplyDigCardRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_DROP, GG_ApplyDropRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_DECLARE, GG_ApplyDeclareRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_LOSE_DECLARE, GG_ApplyLoseDeclareRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_ROOM_STATUS, GG_ApplyRoomStatusRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_TRUSTEE_OFF, GG_ApplyTrusteeOffRequest);
	PARSE_MESSAGE(GG_CMD_APPLY_CLOSE_ACCOUNT_BOX, GG_ApplyCloseAccountBoxRequest);
	PARSE_MESSAGE_END(msg.m_cmd_type);
}

GG_GameArea_Manager::~GG_GameArea_Manager()
{
	close(1);
}

int GG_GameArea_Manager::close(int flags /* = 0 */)
{
	if (flags)
	{
		for (auto iter = m_game_area.begin(); iter != m_game_area.end(); ++iter)
		{
			delete iter->second;
		}
		m_game_area.clear();
	}
	return 0;
}

GG_GameArea* GG_GameArea_Manager::make_gamearea(int area_id)
{
	return new GG_GameArea(area_id);
}

int GG_GameArea_Manager::open(void *args)
{
	TRACE_FUNCATION();
	ACE_OS::sleep(2);
	m_preactor = Game_Process::instance()->get_server()->get_io_reactor();

	m_svr_port = (short)(long)args;

	load_area();

	send_flush_room_alive();
	return 0;
}

int GG_GameArea_Manager::svc(void)
{
	return 0;
}

int GG_GameArea_Manager::handle_input(ACE_HANDLE fd /* = ACE_INVALID_HANDLE */)
{
	TRACE_FUNCATION();
	dispatch_msg();
	return 0;
}

int GG_GameArea_Manager::putq(ACE_Message_Block *mb, ACE_Time_Value *tv)
{
	int ret = ACE_Task <ACE_MT_SYNCH>::putq(mb, tv);
	if (-1 == ret)
	{
		LOG_ERROR("putq failed. q:" + toString(this->msg_queue()->message_count()));
		return ret;
	}

	m_preactor->notify(this, ACE_Event_Handler::READ_MASK);
	return 0;
}

int GG_GameArea_Manager::handle_timeout(const ACE_Time_Value &current_time, const void *act /* = 0 */)
{
	if (act == &m_check_player_timer)
	{
		check_player_online();
		time_t tNow = time(NULL);
		if (m_last_flush_time + GG_GameArea::FLUSH_ROOM_INTERVAL <= tNow)
		{
			
			m_last_flush_time = tNow;
			send_flush_room_alive();
			send_flush_room_status();
		}
	}
	return 0;
}

int GG_GameArea_Manager::load_area(void)
{
	TRACE_FUNCATION();
	std::list<int> lst_area_id;
	if (-1 == get_area_id(lst_area_id))
	{
		LOG_ERROR("get area id failed");
		return -1;
	}

	for (std::list<int>::iterator iter = lst_area_id.begin(); iter != lst_area_id.end(); ++iter)
	{
        LOG_INFO("the area id is " + toString(*iter));
		GG_GameArea *area = make_gamearea(*iter);
		add_game_area(area);
	}

	m_preactor->schedule_timer(this, &m_check_player_timer, ACE_Time_Value(GG_GameArea::CHECK_ONLINE_INTERVAL, 0), ACE_Time_Value(GG_GameArea::CHECK_ONLINE_INTERVAL, 0));
	return 0;
}

int GG_GameArea_Manager::add_game_area(GG_GameArea *game_area)
{
	game_area->reactor(m_preactor);
	if (-1 == game_area->open())
	{
		LOG_ERROR("game area(" + toString(game_area->area_id()) + ") open failed");
		return 0;
	}
	assert(NULL == m_game_area[game_area->area_id()]);
	m_game_area[game_area->area_id()] = game_area;
	return 0;
}

int GG_GameArea_Manager::get_area_id(std::list<int>& lst_area_id)
{
	TRACE_FUNCATION();
	GG_CommQueryRequest request;
	
	request.set_templatename(GG_SQL_GAMESVR_LOAD_ROOM_ID);
	request.add_queryparas(toString(m_svr_port));
	GG_Response response;
	while (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMMON_QUERY,MQ_TRANSMIT_DIRECT,SITE_NAME_ROOMSVR + "." + 
		Game_Process::instance()->getGameName() + ".master",
		request, response))
	{
		LOG_ERROR("get room id failed. wait 10s");
		ACE_OS::sleep(10);
	}

	if (response.m_result != GG_ERROR_SUCCESS)
	{
		LOG_ERROR("get room id failed failed.result:" + toString(response.m_result));
		return -1;
	}
	PBMEssage* pbmsg = response.m_pPBMsg;
	LOG_DEBUG("test_pbmsg:" + toString(pbmsg->ByteSize()));
	const GG_CommQueryResponse& pbresponse = response.ref_pbmsg<GG_CommQueryResponse>();
	for (auto iter = pbresponse.queryresult().begin(); iter != pbresponse.queryresult().end(); ++iter)
	{
		
		assert(iter->dbrow().size() > 0);
		lst_area_id.push_back(atoi(iter->dbrow(0).c_str()));
	}
	return 0;
}

int GG_GameArea_Manager::check_msg(int roomId)
{
	TRACE_FUNCATION();
	
	if (!is_area_exist(roomId))
	{
        LOG_ERROR("area id:" + toString(roomId) + " invalid");
		return -1;
	}

	return 0;
}

bool GG_GameArea_Manager::is_valid_player(int user_id, long session_id)
{
	return 0 == session_id || GamePlayer_Manager::instance()->check_player_session(user_id, session_id);
}

bool GG_GameArea_Manager::is_area_exist(int area_id)
{
	bool is_exist = false;
	is_exist = m_game_area.find(area_id) != m_game_area.end();
	return is_exist;
}

void GG_GameArea_Manager::dispatch_area_msg(const GG_Message &msg)
{
	TRACE_FUNCATION();
	int userId = 0;
	int roomId = 0;
	parse_rid_uid_from_msg(msg, userId, roomId);
	if (-1 == check_msg(roomId))
	{
		msg.send_failed_reason(GG_ERROR_INVALID_ROOMID);
		LOG_ERROR("check msg failed");
		return;
	}

	m_game_area[roomId]->on_recv_msg(msg);
}

int GG_GameArea_Manager::add_new_room(int room_id)
{
	return 0;
}

void GG_GameArea_Manager::close_room(GG_GameArea *area)
{
	TRACE_FUNCATION();
	assert(NULL != area);
	LOG_INFO("close room(" + toString(area->area_id()) + ")");
	auto iter = m_game_area.find(area->area_id());
	assert(iter != m_game_area.end() && area == iter->second);
	if (iter != m_game_area.end())
	{
		m_game_area.erase(iter);
	}
	delete area;
}

short GG_GameArea_Manager::get_gamesvr_port(void)
{
	return m_svr_port;
}

void GG_GameArea_Manager::dispatch_msg(void)
{
	ACE_Message_Block *pBlock = NULL;
	ACE_Time_Value time_out(0,0);
	while (-1 != getq(pBlock, &time_out))
	{
		GG_Message *pMsg = (GG_Message*)pBlock->base();

		switch (pMsg->m_cmd_type)
		{
			dispatch_area_msg(*pMsg);
			break;
		}
		delete pMsg;
		delete pBlock;
		pBlock = NULL;
	}
}

void GG_GameArea_Manager::output_manager_info(void)
{
	LOG_INFO("ps-room_manager(" + toString(this) + ") room(" + toString(m_game_area.size()) + ") queue(" + toString(this->msg_queue()->message_count()) + ")");
}

void GG_GameArea_Manager::check_player_online(void)
{
	for (auto iter = m_game_area.begin(); iter != m_game_area.end(); ++iter)
	{
		iter->second->check_player_online();
	}
}

void GG_GameArea_Manager::send_flush_room_alive(void)
{
    GG_FlushRoomAliveRequest request;   
	request.set_svrport(m_svr_port);
    for (auto iter = m_game_area.begin(); iter != m_game_area.end(); ++iter)
    {
		GG_GameRoomInfo* room = request.add_roominfo();
		room->set_roomid(iter->second->get_area_info().m_area_id);
		room->set_playingcount(iter->second->get_playing_count());
		DEBUG_LOG << "room(" << iter->second->get_area_info().m_area_id << ") send playing_count(" << room->playingcount() << ")";
    }
    MQService::instance()->send_mqrequest(GG_CMD_FLUSH_ROOM_ALIVE, MQ_TRANSMIT_TOPIC, SITE_NAME_ROOMSVR + "." + Game_Process::instance()->getGameName(), request);
}

void GG_GameArea_Manager::send_flush_room_status(void)
{
	GG_FlushRoomStausRequest request;
	request.set_svrport(m_svr_port);
	for (auto iter = m_game_area.begin(); iter != m_game_area.end(); ++iter)
	{
		RoomStatusInfo* room = request.add_roominfo() ;
		room->set_roomid(iter->second->get_area_info().m_area_id);
		room->set_playingcount(iter->second->get_playing_count());
		room->set_status(iter->second->get_game_desk()->get_cur_notify_type());
	}
	MQService::instance()->send_mqrequest(GG_CMD_FLUSH_ROOM_STATUS, MQ_TRANSMIT_TOPIC, SITE_NAME_ROOMSVR + "." + Game_Process::instance()->getGameName(), request);
}
