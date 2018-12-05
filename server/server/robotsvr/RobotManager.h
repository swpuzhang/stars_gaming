#ifndef  BOTMAN_MANAGER_H
#define BOTMAN_MANAGER_H
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>

#include "MongoInfo.h"
#include "UserCommon.h"

class Robot;

using RobotPtr = std::shared_ptr<Robot>;

enum class RobotStatus
{
	ROBOT_IDLE = 0,
	ROBOT_WAIT_WORK,
	ROBOT_WORKING
};

class RobotManager
{
public:
	RobotManager();
	void load_robot();
private:
	bool create_one_robot();
	const static int MAX_WORK_ROBOT = 1000;
	std::vector<std::string> m_device_names;
	std::unordered_map<RobotStatus, std::string> m_robot_status_name;
	std::unordered_map<int, RobotPtr> m_all_robots;
};

#endif
