#ifndef GG_SITEMSG_FACTORY_H__
#define GG_SITEMSG_FACTORY_H__

#include "ggpp/GG_Message_Factory.h"

class GG_SiteMsg_Factory : public GG_Message_Factory
{
public:
	GG_SiteMsg_Factory(void);
	virtual ~GG_SiteMsg_Factory(void);

	virtual void register_cmd_message(void) override;
};

#endif //GG_SITEMSG_FACTORY_H__

