#include "MessageMaker.h"
#include "liblog/Log.h"
#include "Message.h"
#include "MessageBuffer.h"
#include "SystemCmd.pb.h"
#include "SystemMsg.pb.h"

MessageMaker:: MessageMaker()
{
	register_message(SystemCmd::CMD_HEARTBEAT, SystemMsg::HeartbeatRequest(), SystemMsg::HeartbeatResponse());
	register_message(SystemCmd::CMD_COMM_RPC, SystemMsg::RPCRequest(), SystemMsg::RPCResponse());
	register_message(SystemCmd::CMD_SESSION_SILENT, SystemMsg::SessionSilentRequest(), SystemMsg::SessionSilentResponse());
	register_message(SystemCmd::CMD_SESSION_CLOSE, SystemMsg::SessionCloseRequest(), SystemMsg::SessionCloseResponse());
	register_message(SystemCmd::CMD_SHUTDOWN_SESSION, SystemMsg::ShutDownSessionRequest(), SystemMsg::ShutDownSessionResponse());
	register_message(SystemCmd::CMD_SITE_REGISTER_BUS, SystemMsg::SiteRegisterBusRequest(), SystemMsg::SiteRegisterBusResponse());
	register_message(SystemCmd::CMD_SITE_TO_BUS_TRANSMIT, SystemMsg::SiteToBusTransmitRequest(), SystemMsg::SiteToBusTransmitResponse());
	register_message(SystemCmd::CMD_BUS_TO_SITE_TRANSMIT, SystemMsg::BusToSiteTransmitRequest(), SystemMsg::BusToSiteTransmitResponse());
	register_message(SystemCmd::CMD_STREAM_OVER_CTRL, SystemMsg::StreamOverCtrlRequest(), SystemMsg::StreamOverCtrlResponse());
	register_message(SystemCmd::CMD_MQ_TRANSMIT, SystemMsg::MQTransmitRequest(), SystemMsg::MQTransmitResponse());
	register_message(SystemCmd::CMD_SESSION_OPEN, SystemMsg::SessionOpenRequest(), SystemMsg::SessionOpenResponse());
}

PbMessagePtr MessageMaker::make_message(int msg_type, int cmd_type) const
{
	if (msg_type & MsgType::REQUEST_MASK)
	{
		return make_request_message(cmd_type);
	}
	else if (msg_type & MsgType::RESPONSE_MASK)
	{
		return make_response_message(cmd_type);
	}
	else
	{
		return nullptr;
	}
	ERROR_LOG << "make_message cmdtype:" << cmd_type << " type:" << msg_type << " failed";
	return nullptr;
}

PbMessagePtr MessageMaker::make_response_message(int cmd_type) const
{
	auto it = m_cmd_response.find(cmd_type);
	if (it != m_cmd_response.end())
	{
		DEBUG_LOG << "make_response_message cmdtype:" << cmd_type << " success";
		return PbMessagePtr(it->second->New());
	}
	WARN_LOG << "make_response_message cmdtype:" << cmd_type << " failed";
	return nullptr;
}

PbMessagePtr MessageMaker::make_request_message(int cmd_type) const
{
	auto it = m_cmd_request.find(cmd_type);
	if (it != m_cmd_request.end())
	{
		DEBUG_LOG << "make_request_message cmdtype:" << cmd_type << " success";
		return PbMessagePtr(it->second->New());
	}
	WARN_LOG << "make_request_message cmdtype:" << cmd_type << " failed";
	return nullptr;
}

void MessageMaker::register_message(const int cmd_type,  const PbMessage& msg, const PbMessage& response)
{
	m_cmd_names[cmd_type] = cmd_type;
	m_cmd_request[cmd_type] = std::shared_ptr<PbMessage>(msg.New());
	m_cmd_response[cmd_type] =  std::shared_ptr<PbMessage>(response.New());
	DEBUG_LOG << "register_message cmdtype:" << cmd_type;
}