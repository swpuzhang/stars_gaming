#include "JsonParser.h"
#define FIELD_CLASS1(class_name, type1,field1) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
return jv; \
}\
}

#define FIELD_CLASS2(class_name, type1,field1,type2,field2) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
return jv; \
}\
}

#define FIELD_CLASS3(class_name, type1,field1,type2,field2,type3,field3) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
return jv; \
}\
}

#define FIELD_CLASS4(class_name, type1,field1,type2,field2,type3,field3,type4,field4) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
return jv; \
}\
}

#define FIELD_CLASS5(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
return jv; \
}\
}

#define FIELD_CLASS6(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
return jv; \
}\
}

#define FIELD_CLASS7(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
return jv; \
}\
}

#define FIELD_CLASS8(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
return jv; \
}\
}

#define FIELD_CLASS9(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
return jv; \
}\
}

#define FIELD_CLASS10(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
return jv; \
}\
}

#define FIELD_CLASS11(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
return jv; \
}\
}

#define FIELD_CLASS12(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
return jv; \
}\
}

#define FIELD_CLASS13(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
return jv; \
}\
}

#define FIELD_CLASS14(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13,type14,field14) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
type14::FieldValue m_##field14; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } , \
{#field14, type14::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
m_##field14 = jv[#field14].get<type14::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
jv[#field14] = m_##field14; \
return jv; \
}\
}

#define FIELD_CLASS15(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13,type14,field14,type15,field15) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
type14::FieldValue m_##field14; \
type15::FieldValue m_##field15; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } , \
{#field14, type14::JsonType } , \
{#field15, type15::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
m_##field14 = jv[#field14].get<type14::FieldValue>();\
m_##field15 = jv[#field15].get<type15::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
jv[#field14] = m_##field14; \
jv[#field15] = m_##field15; \
return jv; \
}\
}

#define FIELD_CLASS16(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13,type14,field14,type15,field15,type16,field16) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
type14::FieldValue m_##field14; \
type15::FieldValue m_##field15; \
type16::FieldValue m_##field16; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } , \
{#field14, type14::JsonType } , \
{#field15, type15::JsonType } , \
{#field16, type16::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
m_##field14 = jv[#field14].get<type14::FieldValue>();\
m_##field15 = jv[#field15].get<type15::FieldValue>();\
m_##field16 = jv[#field16].get<type16::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
jv[#field14] = m_##field14; \
jv[#field15] = m_##field15; \
jv[#field16] = m_##field16; \
return jv; \
}\
}

#define FIELD_CLASS17(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13,type14,field14,type15,field15,type16,field16,type17,field17) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
type14::FieldValue m_##field14; \
type15::FieldValue m_##field15; \
type16::FieldValue m_##field16; \
type17::FieldValue m_##field17; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } , \
{#field14, type14::JsonType } , \
{#field15, type15::JsonType } , \
{#field16, type16::JsonType } , \
{#field17, type17::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
m_##field14 = jv[#field14].get<type14::FieldValue>();\
m_##field15 = jv[#field15].get<type15::FieldValue>();\
m_##field16 = jv[#field16].get<type16::FieldValue>();\
m_##field17 = jv[#field17].get<type17::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
jv[#field14] = m_##field14; \
jv[#field15] = m_##field15; \
jv[#field16] = m_##field16; \
jv[#field17] = m_##field17; \
return jv; \
}\
}

#define FIELD_CLASS18(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13,type14,field14,type15,field15,type16,field16,type17,field17,type18,field18) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
type14::FieldValue m_##field14; \
type15::FieldValue m_##field15; \
type16::FieldValue m_##field16; \
type17::FieldValue m_##field17; \
type18::FieldValue m_##field18; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } , \
{#field14, type14::JsonType } , \
{#field15, type15::JsonType } , \
{#field16, type16::JsonType } , \
{#field17, type17::JsonType } , \
{#field18, type18::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
m_##field14 = jv[#field14].get<type14::FieldValue>();\
m_##field15 = jv[#field15].get<type15::FieldValue>();\
m_##field16 = jv[#field16].get<type16::FieldValue>();\
m_##field17 = jv[#field17].get<type17::FieldValue>();\
m_##field18 = jv[#field18].get<type18::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
jv[#field14] = m_##field14; \
jv[#field15] = m_##field15; \
jv[#field16] = m_##field16; \
jv[#field17] = m_##field17; \
jv[#field18] = m_##field18; \
return jv; \
}\
}

#define FIELD_CLASS19(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13,type14,field14,type15,field15,type16,field16,type17,field17,type18,field18,type19,field19) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
type14::FieldValue m_##field14; \
type15::FieldValue m_##field15; \
type16::FieldValue m_##field16; \
type17::FieldValue m_##field17; \
type18::FieldValue m_##field18; \
type19::FieldValue m_##field19; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } , \
{#field14, type14::JsonType } , \
{#field15, type15::JsonType } , \
{#field16, type16::JsonType } , \
{#field17, type17::JsonType } , \
{#field18, type18::JsonType } , \
{#field19, type19::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
m_##field14 = jv[#field14].get<type14::FieldValue>();\
m_##field15 = jv[#field15].get<type15::FieldValue>();\
m_##field16 = jv[#field16].get<type16::FieldValue>();\
m_##field17 = jv[#field17].get<type17::FieldValue>();\
m_##field18 = jv[#field18].get<type18::FieldValue>();\
m_##field19 = jv[#field19].get<type19::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
jv[#field14] = m_##field14; \
jv[#field15] = m_##field15; \
jv[#field16] = m_##field16; \
jv[#field17] = m_##field17; \
jv[#field18] = m_##field18; \
jv[#field19] = m_##field19; \
return jv; \
}\
}

#define FIELD_CLASS20(class_name, type1,field1,type2,field2,type3,field3,type4,field4,type5,field5,type6,field6,type7,field7,type8,field8,type9,field9,type10,field10,type11,field11,type12,field12,type13,field13,type14,field14,type15,field15,type16,field16,type17,field17,type18,field18,type19,field19,type20,field20) class class_name \
{\
public:\
type1::FieldValue m_##field1; \
type2::FieldValue m_##field2; \
type3::FieldValue m_##field3; \
type4::FieldValue m_##field4; \
type5::FieldValue m_##field5; \
type6::FieldValue m_##field6; \
type7::FieldValue m_##field7; \
type8::FieldValue m_##field8; \
type9::FieldValue m_##field9; \
type10::FieldValue m_##field10; \
type11::FieldValue m_##field11; \
type12::FieldValue m_##field12; \
type13::FieldValue m_##field13; \
type14::FieldValue m_##field14; \
type15::FieldValue m_##field15; \
type16::FieldValue m_##field16; \
type17::FieldValue m_##field17; \
type18::FieldValue m_##field18; \
type19::FieldValue m_##field19; \
type20::FieldValue m_##field20; \
const Json m_jv_temp = {\
{#field1, type1::JsonType } , \
{#field2, type2::JsonType } , \
{#field3, type3::JsonType } , \
{#field4, type4::JsonType } , \
{#field5, type5::JsonType } , \
{#field6, type6::JsonType } , \
{#field7, type7::JsonType } , \
{#field8, type8::JsonType } , \
{#field9, type9::JsonType } , \
{#field10, type10::JsonType } , \
{#field11, type11::JsonType } , \
{#field12, type12::JsonType } , \
{#field13, type13::JsonType } , \
{#field14, type14::JsonType } , \
{#field15, type15::JsonType } , \
{#field16, type16::JsonType } , \
{#field17, type17::JsonType } , \
{#field18, type18::JsonType } , \
{#field19, type19::JsonType } , \
{#field20, type20::JsonType } }; \
void from_json(const Json& jv) \
{\
m_##field1 = jv[#field1].get<type1::FieldValue>();\
m_##field2 = jv[#field2].get<type2::FieldValue>();\
m_##field3 = jv[#field3].get<type3::FieldValue>();\
m_##field4 = jv[#field4].get<type4::FieldValue>();\
m_##field5 = jv[#field5].get<type5::FieldValue>();\
m_##field6 = jv[#field6].get<type6::FieldValue>();\
m_##field7 = jv[#field7].get<type7::FieldValue>();\
m_##field8 = jv[#field8].get<type8::FieldValue>();\
m_##field9 = jv[#field9].get<type9::FieldValue>();\
m_##field10 = jv[#field10].get<type10::FieldValue>();\
m_##field11 = jv[#field11].get<type11::FieldValue>();\
m_##field12 = jv[#field12].get<type12::FieldValue>();\
m_##field13 = jv[#field13].get<type13::FieldValue>();\
m_##field14 = jv[#field14].get<type14::FieldValue>();\
m_##field15 = jv[#field15].get<type15::FieldValue>();\
m_##field16 = jv[#field16].get<type16::FieldValue>();\
m_##field17 = jv[#field17].get<type17::FieldValue>();\
m_##field18 = jv[#field18].get<type18::FieldValue>();\
m_##field19 = jv[#field19].get<type19::FieldValue>();\
m_##field20 = jv[#field20].get<type20::FieldValue>();\
}\
Json to_json() \
{\
Json jv; \
jv[#field1] = m_##field1; \
jv[#field2] = m_##field2; \
jv[#field3] = m_##field3; \
jv[#field4] = m_##field4; \
jv[#field5] = m_##field5; \
jv[#field6] = m_##field6; \
jv[#field7] = m_##field7; \
jv[#field8] = m_##field8; \
jv[#field9] = m_##field9; \
jv[#field10] = m_##field10; \
jv[#field11] = m_##field11; \
jv[#field12] = m_##field12; \
jv[#field13] = m_##field13; \
jv[#field14] = m_##field14; \
jv[#field15] = m_##field15; \
jv[#field16] = m_##field16; \
jv[#field17] = m_##field17; \
jv[#field18] = m_##field18; \
jv[#field19] = m_##field19; \
jv[#field20] = m_##field20; \
return jv; \
}\
}
