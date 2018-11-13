#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H
#include <unordered_map>
#include <list>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "libtools/Types.h"
#include "libmessage/MessageType.h"


#define CMD_HANDER 
template< typename MsgTag>
using MSG_HANDLER_FUNC = std::function<void(const MessagePtr<MsgTag>&)>;

#define TASK_BIND(fun) (std::bind(&SELF_TYPE::fun, this, PLACEHOLDER::_1))


//消息CMD映射关系注册开始
/*#define CMD_REGISTER_BEGIN()	\
	public:\
	virtual int init_cmd_mapping(std::list<CMD_TYPE>& lstCMD)\
{	\
	lstCMD.push_back(CMD_UNKNOWN);\
	this->register_cmd_handler(CMD_UNKNOWN, TASK_BIND(TaskManager, handle_cmd_unknown));\
	//向TaskManager注册消息CMD
#define CMD_REGISTER(CMD, CMD_HANDLER)	\
	lstCMD.push_back(CMD);\
	this->register_cmd_handler(CMD, CMD_HANDLER);


//消息CMD映射关系注册结束
#define CMD_REGISTER_END()	\
	return 0;\
}*/

template<typename MsgTag>
class TaskManager
{
public:
	using CMD_TYPE = int ;
	using SELF_TYPE = TaskManager<MsgTag> ;
	TaskManager();
	void register_msg_handler(const CMD_TYPE& cmd, MSG_HANDLER_FUNC<MsgTag> hander);
	virtual void handle_message( const MessagePtr<MsgTag>&);
	
protected:
	virtual void on_unknown_msg(const MessagePtr<MsgTag>& msg);
	virtual void on_session_close(const MessagePtr<MsgTag>& msg);
	virtual void on_session_open(const MessagePtr<MsgTag>& msg);
	virtual void on_heart_beat(const MessagePtr<MsgTag>& msg);
	void do_handle_message(MessagePtr<MsgTag> msg);
private:
	std::unordered_map<CMD_TYPE, MSG_HANDLER_FUNC<MsgTag>> m_cmd_handler;
};

using TcpTask = TaskManager<TcpTag>;
using MqTask = TaskManager<MqTag>;

#include "TaskManager.inl"
#endif