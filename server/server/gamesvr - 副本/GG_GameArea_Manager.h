#ifndef GG_GAMEAREA_MANAGER_H__
#define GG_GAMEAREA_MANAGER_H__

#include <unordered_map>
#include "ace/Singleton.h"
#include "ggpp/GG_Message.h"
#include "ace/Task.h"
#include "ace/Reactor.h"
#include <list>
#include <vector>

class GG_GameArea;

class GG_GameArea_Manager : public ACE_Task <ACE_MT_SYNCH>
{
public:
	GG_GameArea_Manager(void);
	virtual ~GG_GameArea_Manager(void);

	virtual int open(void *args  = 0 );

	virtual int svc(void);

	virtual int close(int flags /* = 0 */);

	virtual int handle_timeout(const ACE_Time_Value &current_time, const void *act /* = 0 */);

	virtual int handle_input(ACE_HANDLE fd /* = ACE_INVALID_HANDLE */);

	virtual int putq(ACE_Message_Block *mb, ACE_Time_Value *tv = NULL);

	void dispatch_area_msg(const GG_Message &msg);

	int add_new_room(int room_id);
	
	void close_room(GG_GameArea *area);

	short get_gamesvr_port(void);

	void output_manager_info(void);

    void send_flush_room_alive(void);

	void parse_rid_uid_from_msg(const GG_Message &msg, int& userId, int& roomId);

	void send_flush_room_status(void);
protected:
	
protected:
	int load_area(void);

	virtual int get_area_id(std::list<int>& lst_area_id);

	void dispatch_msg(void);

	virtual GG_GameArea* make_gamearea(int area_id);

	virtual int check_msg(int roomId);

	virtual int add_game_area(GG_GameArea *game_area);

	bool is_area_exist(int area_id);

	bool is_valid_player(int user_id, long session_id);

	void check_player_online(void);

private:

	std::unordered_map<int, GG_GameArea*> m_game_area;
	ACE_Reactor *m_preactor;
	short m_svr_port;
	bool m_is_running;
	long m_check_player_timer;
	time_t m_last_flush_time;
	std::vector<int> m_match_cards_num_priority;
};

typedef ACE_Singleton<GG_GameArea_Manager, ACE_Thread_Mutex> GameArea_Manager;

//////////////////////////////////////////////////////////////////////////
#define PARSE_MESSAGE_BEGIN(cmd_type) \
	TRACE_FUNCATION();\
	switch (cmd_type) \
	{ 
#define PARSE_MESSAGE(cmd_type, class_name)\
	case cmd_type:\
	{\
		const class_name& pbmsg = msg.ref_pbmsg<class_name>();\
		userId = pbmsg.userid();\
		roomId = pbmsg.roomid();\
		break;\
	}

#define PARSE_MESSAGE_END(cmd_type) \
	default:\
	{\
		LOG_ERROR("unknown cmd_type(" + toString(cmd_type) + ")");\
		userId = 0;\
		roomId = 0;\
		break;\
	}\
	}\


#endif  //GG_GAMEAREA_MANAGER_H__

