#ifndef USER_COMMON
#define  USER_COMMON
#include "libtools/JsonParser.h"
#include "FiledClass.h"
FIELD_CLASS2(AccountInfo,
	StringType, account,
	IntType, user_id);

FIELD_CLASS13(UserInfo,
	StringType, account,
	StringType, device_code,
	StringType, device_name,
	IntType, app_type,
	IntType, user_id,
	IntType, user_sex,
	StringType, user_name,
	StringType, header_url,
	IntType, user_level,
	IntType, user_cur_exp,
	IntType, need_exp,
	IntType, last_login_time,
	IntType, last_offline_time);


#endif
