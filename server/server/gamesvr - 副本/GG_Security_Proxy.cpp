#include "GG_Security_Proxy.h"
#include "GG_SecurityMsg_Factory.h"
#include "libggppss/GG_Server.h"
#include "logger/ilog.h"
#include "ggpp/GG_Message_Factory.h"
#include "json/json.h"
#include "GG_SecurityTask_Manager.h"

extern Json::Value parse_json_string(const std::string& json_string);
extern std::string json_to_string(const Json::Value& json_value);

GG_Security_Proxy::GG_Security_Proxy(void)
	: GG_Service(new GG_SecurityMsg_Factory, true), m_pClient(NULL), m_pTaskManager(NULL)
{
	m_pTaskManager = new GG_SecurityTask_Manager();
	this->register_task_manager(m_pTaskManager);
	m_pClient = new GG_TPClient(ShareClientReactor::instance()->get_reactor());
	this->set_service_name("security-proxy");
}


GG_Security_Proxy::~GG_Security_Proxy(void)
{
	shutdown();
	delete m_pClient;
	delete m_pTaskManager;
}

int GG_Security_Proxy::open(const std::string& host_url)
{
	m_pClient->register_service(this);
	return m_pClient->open(host_url, true);
}

void GG_Security_Proxy::shutdown(void)
{
	m_pClient->shutdown();
}
