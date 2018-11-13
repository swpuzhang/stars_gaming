#ifndef GG_MONEY_PROXY_H__
#define GG_MONEY_PROXY_H__

#include "ace/Singleton.h"
#include "libggppcc/GG_TPClient.h"
#include "ggpp/GG_Service.h"
#include "ace/Singleton.h"
#include "libggppcc/GG_TPClient_Reactor.h"
#include "json/json.h"

class GG_Server;
class GG_Service;

class GG_Money_Proxy
{
public:
	GG_Money_Proxy(void);
	virtual ~GG_Money_Proxy(void);

	int open(const std::string& host_url);

	void shutdown(void);

    int get_user_money(int user_id, Json::Value& money_info);

	int deal_lose_account(int user_id, GG_INT64 m_cur_carry, GG_INT64 buy_chips, GG_INT64 lose_chips, 
		 int room_id, const std::string& game_id, Json::Value& money_info);

	int deduct_blind(const std::vector<int>& user_ids, const GG_INT64 small_blind, 
		int room_id, const std::string& game_id, Json::Value& jresp ); //扣费, 先扣携带， 在扣底注

	int timely_account(int winner_id, int loser_id, GG_INT64 chips_count, int room_id, std::string& game_id, Json::Value& jresp);
	int match_over_account(const Json::Value& losers,const Json::Value& winers, const std::string& game_id,
		const int room_id, const GG_INT64 buy_in, const int tips_persent, Json::Value& jresp);

	int add_user_chips(int user_id, GG_INT64 add_count, int add_reason, Json::Value& jrsp);

	int add_user_carry(int user_id, GG_INT64 add_count, int add_reason, Json::Value& jresp);

	int bet_chips(int user_id, GG_INT64 bet_count, int add_reason, Json::Value& jresp);
private:
};

typedef ACE_Singleton<GG_Money_Proxy, ACE_Thread_Mutex> MoneyProxy; 
typedef ACE_Singleton<GG_TPClient_Reactor, ACE_Thread_Mutex> ShareClientReactor; 

#endif //GG_MONEY_PROXY_H__

