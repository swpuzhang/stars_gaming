db.c_user_id.update({}, {"$setOnInsert" : {"counter" : 10210550}});
db.c_user_money.ensureIndex({"user_id" : 1}, {"unique" : true})
db.c_user_info.ensureIndex({"user_id" : 1}, {"unique" : true})
db.c_user_info.ensureIndex({"account" : 1})
