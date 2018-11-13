#include "GG_Money_Proxy.h"
#include "libggppss/GG_Server.h"
#include "logger/ilog.h"
#include "GG_GameArea.h"
#include "GG_SiteMsg_Factory.h"
#include "ggpp/GG_SiteName.h"
#include "libggppcc/GG_MQService.h"
#include "GG_SystemMsg.pb.h"
#include "GG_SystemCmd.pb.h"
#include "json/value.h"
using namespace GG_SystemCmd;
using namespace GG_SystemMsg;

GG_Money_Proxy::GG_Money_Proxy(void)
{

}


GG_Money_Proxy::~GG_Money_Proxy(void)
{
	shutdown();

}

int GG_Money_Proxy::open(const std::string& host_url)
{
	
}

void GG_Money_Proxy::shutdown(void)
{

}


int GG_Money_Proxy::get_user_money(int user_id, Json::Value& money_info)
{
    TRACE_FUNCATION();
    GG_RPCRequest request;
    Json::Value jreq;
    jreq[RPC_METHOD] = "get_user_money";
    jreq[RPC_PARAMETER]["user_id"] = user_id;
    request.set_jsonstring(json_to_string(jreq));
    GG_Response response;
    if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
    {
        LOG_ERROR("send get_user_money msg failed");
        return -1;
    }
	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();
    Json::Value jresp = parse_json_string(pbresponse.jsonstring());

    if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
    {
        LOG_ERROR("get_user_money response failed");
        return -1;
    }

    money_info = jresp[RPC_PARAMETER];
    return 0;
}
///Modify by Jim @15.08.14
int GG_Money_Proxy::add_user_chips(int user_id, GG_INT64 add_count, int add_reason, Json::Value& jresp)
{
	TRACE_FUNCATION();
	GG_RPCRequest request;
	Json::Value jreq;
	jreq[RPC_METHOD] = "add_user_chips";
	jreq[RPC_PARAMETER]["user_id"] = user_id;
	jreq[RPC_PARAMETER]["add_count"] = toString(add_count);
	jreq[RPC_PARAMETER]["add_reason"] = add_reason;
	request.set_jsonstring(json_to_string(jreq));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send get_user_money msg failed");
		return -1;
	}

	if (!response.success())
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();
	jresp = parse_json_string(pbresponse.jsonstring());

	if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	return 0;
}


int GG_Money_Proxy::deal_lose_account(int user_id, GG_INT64 m_cur_carry, GG_INT64 buy_chips, GG_INT64 lose_chips, 
									  int room_id, const std::string& game_id, Json::Value& money_info)
{
	TRACE_FUNCATION();
	GG_RPCRequest request;
	Json::Value jreq;
	jreq[RPC_METHOD] = "deal_lose_account";
	jreq[RPC_PARAMETER]["user_id"] = user_id;
	jreq[RPC_PARAMETER]["room_id"] = room_id;
	jreq[RPC_PARAMETER]["game_id"] = game_id;
	jreq[RPC_PARAMETER]["carry"] = toString(m_cur_carry);
	jreq[RPC_PARAMETER]["lose_chips"] = toString(lose_chips);
	jreq[RPC_PARAMETER]["buy_chips"] = toString(buy_chips);
	request.set_jsonstring(json_to_string(jreq));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send get_user_money msg failed");
		return -1;
	}

	if (!response.success())
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();

	Json::Value jresp = parse_json_string(pbresponse.jsonstring());

	if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
	{
		LOG_ERROR("deal_lose_account response failed");
		return -1;
	}
	money_info = jresp[RPC_PARAMETER];
	return 0;
}


int GG_Money_Proxy::deduct_blind(const std::vector<int>& user_ids, const GG_INT64 small_blind, int room_id, 
								 const std::string& game_id, Json::Value& jresp )
{
	TRACE_FUNCATION();
	GG_RPCRequest request;
	Json::Value jreq;
	jreq[RPC_METHOD] = "deduct_blind";
	jreq[RPC_PARAMETER]["small_blind"] = toString(small_blind);
	jreq[RPC_PARAMETER]["room_id"] = room_id;
	jreq[RPC_PARAMETER]["game_id"] = game_id;
	for (std::vector<int>::const_iterator it = user_ids.begin(); it != user_ids.end(); ++it)
	{
		Json::Value jv(*it);
		jreq[RPC_PARAMETER]["user_ids"].append(jv); //Json::Int(*it));
	}
	request.set_jsonstring(json_to_string(jreq));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send get_user_money msg failed");
		return -1;
	}

	if (!response.success())
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();

	jresp = parse_json_string(pbresponse.jsonstring());

	if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
	{
		LOG_ERROR("deduct_chips response failed");
		return -1;
	}

	return 0;
}

int GG_Money_Proxy::timely_account(int winner_id, int loser_id, GG_INT64 chips_count,int room_id, std::string& game_id, Json::Value& jresp)
{

	TRACE_FUNCATION();
	GG_RPCRequest request;
	Json::Value jreq;
	jreq[RPC_METHOD] = "timely_account";
	jreq[RPC_PARAMETER]["winner_user_id"] = winner_id;
	jreq[RPC_PARAMETER]["loser_user_id"] = loser_id;
	jreq[RPC_PARAMETER]["chip_count"] = toString(chips_count);
	jreq[RPC_PARAMETER]["room_id"] = room_id;
	jreq[RPC_PARAMETER]["game_id"] = game_id;

	request.set_jsonstring(json_to_string(jreq));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send get_user_money msg failed");
		return -1;
	}

	if (!response.success())
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();

	jresp = parse_json_string(pbresponse.jsonstring());

	if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}
	return 0;
}

int GG_Money_Proxy::match_over_account(const Json::Value& losers,const Json::Value& winers, const std::string& game_id,
								   const int room_id, const GG_INT64 buy_in, const int tips_persent, Json::Value& jresp)
{

	TRACE_FUNCATION();
	GG_RPCRequest request;
	Json::Value jreq;
	jreq[RPC_METHOD] = "match_over_account";
	jreq[RPC_PARAMETER]["losers"] = losers;
	jreq[RPC_PARAMETER]["winners"] = winers;
	jreq[RPC_PARAMETER]["room_id"] = room_id;
	jreq[RPC_PARAMETER]["game_id"] = game_id;
	jreq[RPC_PARAMETER]["buy_in"] = toString(buy_in);
	jreq[RPC_PARAMETER]["tips_persent"] = tips_persent;

	request.set_jsonstring(json_to_string(jreq));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send get_user_money msg failed");
		return -1;
	}

	if (!response.success())
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();

	jresp = parse_json_string(pbresponse.jsonstring());

	if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}
	return 0;
}

int GG_Money_Proxy::add_user_carry(int user_id, GG_INT64 add_count, int add_reason, Json::Value& jresp)
{
	TRACE_FUNCATION();
	GG_RPCRequest request;
	Json::Value jreq;
	jreq[RPC_METHOD] = "add_user_carry";
	jreq[RPC_PARAMETER]["user_id"] = user_id;
	jreq[RPC_PARAMETER]["add_count"] = toString(add_count);
	jreq[RPC_PARAMETER]["add_reason"] = add_reason;
	request.set_jsonstring(json_to_string(jreq));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send get_user_money msg failed");
		return -1;
	}

	if (!response.success())
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();

	jresp = parse_json_string(pbresponse.jsonstring());

	if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
	{
		LOG_ERROR("add_user_carry response failed");
		return -1;
	}
	return 0;
}

int GG_Money_Proxy::bet_chips(int user_id, GG_INT64 bet_count, int add_reason, Json::Value& jresp)
{
	TRACE_FUNCATION();
	GG_RPCRequest request;
	Json::Value jreq;
	jreq[RPC_METHOD] = "bet_chips";
	jreq[RPC_PARAMETER]["user_id"] = user_id;
	jreq[RPC_PARAMETER]["add_count"] = toString(bet_count);
	jreq[RPC_PARAMETER]["add_reason"] = add_reason;
	request.set_jsonstring(json_to_string(jreq));
	GG_Response response;
	if (-1 == MQService::instance()->send_mqrequest(GG_CMD_COMM_RPC, MQ_TRANSMIT_DIRECT, SITE_NAME_MONEYSVR, request, response, ACE_Time_Value(3, 0)))
	{
		LOG_ERROR("send get_user_money msg failed");
		return -1;
	}

	if (!response.success())
	{
		LOG_ERROR("add_user_chips response failed");
		return -1;
	}

	const GG_RPCResponse& pbresponse = response.ref_pbmsg<GG_RPCResponse>();

	jresp = parse_json_string(pbresponse.jsonstring());

	if (jresp[RPC_RESULT].asString() != RPC_SUCCESS)
	{
		LOG_ERROR("bet_chips response failed");
		return -1;
	}
	return 0;
}