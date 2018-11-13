#pragma once
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"

class TcpSession;

class SessionManager
{
public:
	virtual void close_session(const std::shared_ptr<TcpSession>& sessoin) = 0;
};