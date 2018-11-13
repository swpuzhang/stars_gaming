#ifndef GG_SECURITY_FACTORY_H__
#define GG_SECURITY_FACTORY_H__

#include "ggpp/GG_Message_Factory.h"

class GG_SecurityMsg_Factory : public GG_Message_Factory
{
public:
	GG_SecurityMsg_Factory(void);
	virtual ~GG_SecurityMsg_Factory(void);

	virtual GG_Message* make_request(int cmd_type);

	virtual GG_Message* make_response(int cmd_type);

	virtual std::string get_cmd_type_name(int cmd_type);
};

#endif //GG_SECURITY_FACTORY_H__
