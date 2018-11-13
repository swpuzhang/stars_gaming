#ifndef GG_GAME_PROCESS_H__
#define GG_GAME_PROCESS_H__

#include "libggppss/GG_Multi_Process.h"
#include "basetool/XmlConfig.h"
#include "ace/Singleton.h"
#include <list>
#include "json/json.h"
#include "GG_GameTask_Manager.h"
#include "libggppcc/GG_SiteService.h"
#include "ggpp/GG_Message.h"

class GG_Server;
class GG_Service;
class GG_GameTask_Manager;

class GG_Game_Process : public GG_Multi_Process
{
public:
	GG_Game_Process(void);
	virtual ~GG_Game_Process(void);

	int run(const std::string& config_path, int argc, char *argv[]);

	const CXmlConfig& get_config(void);

	GG_Service* get_service(void);

	GG_Server* get_server(void);

	tm local_time(time_t t);

	GG_GameTask_Manager* get_game_task_manager(void);

    std::string get_my_hostname(void);
	
protected:
	virtual int master_main(int sub_count, int argc, char *argv[]);

	virtual int subprogram_main(int argc, char *argv[]);

	void get_port_list(std::list<uint16_t> &port_list);

private:
	CXmlConfig m_config;
	GG_Server *m_pServer;
	GG_Service *m_pService;
	GG_GameTask_Manager *m_pGameTaskManager;
    std::string m_my_hostname;
};

typedef ACE_Singleton<GG_Game_Process, ACE_Thread_Mutex> Game_Process; 

Json::Value parse_json_string(const std::string& json_string);
std::string json_to_string(const Json::Value& json_value);

#endif //GG_GAME_PROCESS_H__




