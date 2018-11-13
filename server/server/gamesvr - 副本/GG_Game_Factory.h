#ifndef GG_GAME_FACTORY_H__
#define GG_GAME_FACTORY_H__

#include "ggpp/GG_Message_Factory.h"

class GG_Game_Factory : public GG_Message_Factory
{
public:
	GG_Game_Factory(void);
	virtual ~GG_Game_Factory(void);
protected:
	virtual void register_cmd_message(void);
};

#endif  //GG_GAME_FACTORY_H__

