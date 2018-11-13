#include "Player.h"
#include "GameRoom.h"
#include "GameMsg.pb.h"
#include "GameCmd.pb.h"
#include "libserver/TcpSession.h"


Player::Player() : m_online_check_time(time(NULL)), m_user_id(-1)
{

}

void Player::set_status(int status)
{
	
}


std::string Player::get_status_name(int status) const
{

	std::string status_name;

	if (status & PLAYER_TRUSTEED)
	{
		status_name += "PLAYER_TRUSTEED|";
	}
	if (status & PLAYER_OFFLINE)
	{
		status_name += "PLAYER_OFFLINE|";
	}
	status &= PLAYER_MASK;

	switch (status)
	{
	case PLAYER_WAIT_NEXT_GAME:
	{
		status_name += "PLAYER_WAIT_NEXT_GAME";
		break;
	}
	case PLAYER_UNREADY:
	{
		status_name += "PLAYER_UNREADY";
		break;
	}
	case PLAYER_PLAYING:
	{
		status_name += "PLAYER_PLAYING";
		break;
	}
	case PLAYER_DROP:
	{
		status_name += "PLAYER_DROP";
		break;
	}
	case PLAYER_STAND_UP:
	{
		status_name += "PLAYER_STAND_UP";
		break;
	}
	default:
	{
		status_name += "UNKNOWN:" + std::to_string(status);
		break;
	}
	}
	return status_name;
}

int Player::get_status(void) const
{
	return m_cur_status;
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

