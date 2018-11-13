#ifndef AES_BT_H__
#define AES_BT_H__

#include "BaseToolConfig.h"

void  AES_ecb128_encrypt(const unsigned char *in, size_t length, unsigned char *out, const char *key);
void  AES_ecb128_decrypt(const unsigned char *in, size_t length, unsigned char *out, const char *key);
std::string  AES_ecb128_encrypt(const std::string& in);
std::string  AES_ecb128_decrypt(const std::string& in);
std::string  AES_ecb128_encrypt(const std::string& in, const std::string& key);
std::string  AES_ecb128_decrypt(const std::string& in, const std::string& key);
void  AES_ecb128_decrypt(const char *in, size_t length, char *out, size_t& out_len, const char *key_seed);



#endif //AES_BT_H__

