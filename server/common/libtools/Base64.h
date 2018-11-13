#ifndef BASE64_H_
#define BASE64_H_

#include <string>
#include "BaseToolConfig.h"

enum BASE64_CH_SET { BASE64_CH_NORMAL = 0, BASE64_CH_SECRET };

std::string  Base64Encode(const char* src, size_t length);

void  Base64Encode(const char* src, size_t src_length, char* dest, size_t *encode_length = NULL);

void  Base64Decode(const char* src, size_t src_length, char* dest, size_t *decode_length = NULL);

int  Base64Decode(const std::string& strEn, char *&buf, size_t &length);

std::string  Base64Encode(const char* buf, int length, int chSet/* = BASE64_CH_NORMAL*/);

int  Base64Decode(const std::string& strEn, char *&buf, int &length, int chSet/* = BASE64_CH_NORMAL*/);



#endif  //BASE64_H_

