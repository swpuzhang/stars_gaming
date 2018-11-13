#ifndef GG_Security_Proxy_H__
#define GG_Security_Proxy_H__

#include "ace/Singleton.h"
#include "libggppcc/GG_TPClient.h"
#include "ggpp/GG_Service.h"
#include "ace/Singleton.h"
//#include "GG_SvrInteract_Message.h"
#include "libggppcc/GG_TPClient_Reactor.h"
#include "json/json.h"

class GG_Server;
class GG_Service;

class GG_Security_Proxy : public GG_Service
{
public:
	GG_Security_Proxy(void);
	virtual ~GG_Security_Proxy(void);

	int open(const std::string& host_url);

	void shutdown(void);

private:
	GG_Client *m_pClient;
	GG_Task_Manager *m_pTaskManager;
};

typedef ACE_Singleton<GG_Security_Proxy, ACE_Thread_Mutex> SecurityProxy; 
typedef ACE_Singleton<GG_TPClient_Reactor, ACE_Thread_Mutex> ShareClientReactor; 


#endif //GG_Security_Proxy_H__
