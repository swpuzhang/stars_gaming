#include "Player.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"
#include "libserver/TcpSession.h"


Player::Player() : m_online_check_time(time(NULL)), m_user_id(-1)
{

}

void Player::session_close()
{
	m_session.reset();
}

bool Player::send_request(int cmd_type, const PbMessagePtr &msg)
{
	if (!m_session)
	{
		return false;
	}
	m_session->send_message(cmd_type, msg);
	return true;
}

bool Player::send_request(const TcpMsgPtr &msg)
{
	if (!m_session)
	{
		return false;
	}
	m_session->send(msg);
	return true;
}

