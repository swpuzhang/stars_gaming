#ifndef BASIC_MESSAGE_MAKER_H
#define BASIC_MESSAGE_MAKER_H

#include "libtools/Types.h"
#include <string>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "google/protobuf/descriptor.h"


class MessageMaker
{
public:
	MessageMaker();
	PbMessagePtr make_message(int msg_type, int cmd_type) const;
	void register_message(const int cmd_type,  const PbMessage& request,
		const PbMessage& response);
	PbMessagePtr make_request_message(int cmd_type) const ;
	PbMessagePtr make_response_message(int cmd_type) const;
	virtual  ~MessageMaker() {};
protected:

	std::unordered_map<int, PbMessagePtr> m_cmd_request;
	std::unordered_map<int, PbMessagePtr> m_cmd_response;
	std::unordered_map<int, std::string> m_cmd_names;
};
#endif