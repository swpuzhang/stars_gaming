#include "RobotManager.h"
#include "libresource/Mongodb.h"
#include "libresource/Redis.h"
#include "LanchProcess.h"

RobotManager::RobotManager()
{
	m_device_names.push_back("20F6A05FCD (LENOVO)");
	m_device_names.push_back("90DR000ACD (LENOVO)");
	m_device_names.push_back("BlackBerry BBC100-1");
	m_device_names.push_back("BlackBerry");
	m_device_names.push_back("HUAWEI  HUAWEI MLA-AL10");
	m_device_names.push_back("HUAWEI  HUAWEI BLA-AL00");
	m_device_names.push_back("HUAWEI  HUAWEI BLN-AL40");
	m_device_names.push_back("HUAWEI  HUAWEI CHM-TL00H");
	m_device_names.push_back("HUAWEI  HUAWEI KNT-AL20");
	m_device_names.push_back("Meizu m1 metal");
	m_device_names.push_back("Meizu M5");
	m_device_names.push_back("Meizu M6 Note");
	m_device_names.push_back("OnePlus ONEPLUS A3010");
	m_device_names.push_back("OnePlus3");
	m_device_names.push_back("OPPO A1601");
	m_device_names.push_back("OPPO A51w");
	m_device_names.push_back("OPPO CPH1613");
	m_device_names.push_back("OPPO CPH1717");
	m_device_names.push_back("OPPO OPPO A3");
	m_device_names.push_back("OPPO OPPO A37m");
	m_device_names.push_back("OPPO OPPO R11");
	m_device_names.push_back("OPPO OPPO R11");
	m_device_names.push_back("samsung GT-I8190");
	m_device_names.push_back("samsung GT-I8262");
	m_device_names.push_back("samsung SM-A510F");
	m_device_names.push_back("samsung SM-G610F");
	m_device_names.push_back("samsung SM-G935F");
	m_device_names.push_back("samsung SM-G955N");
	m_device_names.push_back("samsung SM-J111F");
	m_device_names.push_back("samsung SM-J200G");
	m_device_names.push_back("samsung SM-J500G");
	m_device_names.push_back("samsung SM-J530Y");
	m_device_names.push_back("samsung SM-N920T");
	m_device_names.push_back("vivo vivo 1601");
	m_device_names.push_back("Vostro 3900");
	m_device_names.push_back("Xiaomi MI 5X");
	m_device_names.push_back("Xiaomi MI 6");
	m_device_names.push_back("Xiaomi Redmi 4A");
	m_device_names.push_back("Xiaomi Redmi 4X");
	m_device_names.push_back("Xiaomi Redmi Note 5A ");
	m_device_names.push_back("Xiaomi Redmi Note 5A Prime");

	m_robot_status_name[RobotStatus::ROBOT_IDLE] = "idle";
	m_robot_status_name[RobotStatus::ROBOT_WAIT_WORK] = "wait_work";
	m_robot_status_name[RobotStatus::ROBOT_WORKING] = "working";
}

void RobotManager::load_robot()
{
	//db中查询出is_robot = 1的数据
	mongocxx::database db = MongodbInstance::get_mutable_instance().get_client();
	mongocxx::collection  collect = db[USER_INFO_COLLECT];
	mongocxx::cursor find_result = collect.find({ MGDocument{}  << "is_robot" << -1 << MGFinalize });
	for (auto&& doc : find_result)
	{
		UserInfo one_info;
		one_info.from_json(Json::parse(bsoncxx::to_json(doc)));
		auto robot_ptr = std::make_shared<Robot>(LanchInstance::get_mutable_instance().get_io_loop()->get_next_loop(),
			LanchInstance::get_mutable_instance().get_client_loop()->get_next_loop(), one_info.m_user_id);
		m_all_robots[one_info.m_user_id] = robot_ptr;


	}
}