#ifndef BASE_TOOL_CONFIG_H__
#define BASE_TOOL_CONFIG_H__


#include <string>
std::string  bin2hex(const unsigned char *p, size_t len);
void  bin2hex(char *to, const unsigned char *p, size_t len);
void  hex2bin(unsigned char *to, const char *p);
void SimpLeSiftencode(const char* src, int srcLen, int shiftBit, char* des);
void SimpLeSiftdecode(const char* src, int srcLen, int shiftBit, char* des);




#endif  //BASE_TOOL_CONFIG_H__
