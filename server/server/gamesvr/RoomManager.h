#pragma once
#include "libtools/AsioTypes.h"
#include "libtools/Types.h"
#include "libtools/IoLoop.h"
#include "libmessage/Message.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <boost/serialization/singleton.hpp>


class GameRoom;
using GAMEROOM_PTR = std::shared_ptr<GameRoom>;

class RoomManager
{
public:
	void open(TY_UINT16 svr_port, IoLoop* io_loop);
	void dispach_room_msg(const TcpMsgPtr& msg) const;
	std::vector<int> get_room_id();
	void add_room(int room_id);
	IoLoop& get_io_loop() { return *m_io_loop; }
private:
	bool check(int user_id, int room_id) const;
	std::unordered_map<int, GAMEROOM_PTR> m_game_room;
	TY_UINT16 m_svr_port;
	IoLoop* m_io_loop;
};

using RoomManagerInstance = boost::serialization::singleton<RoomManager>;