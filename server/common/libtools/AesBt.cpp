#include "AesBt.h"
#include "AesCore.h"
#include "MD5.h"
#include "string.h"
#include <assert.h>
#include "Base64.h"

static std::string get_key(void)
{
	std::string seed = "safsdfsdfkjl-s-123lkfls";
	std::string key = MD5_32(seed.c_str(), seed.size());
	return key.substr(0, 16);
}

static std::string get_key(const std::string& seed)
{
	std::string key = MD5_32(seed.c_str(), seed.size());
	return key.substr(0, 16);
}

void AES_ecb128_encrypt(const unsigned char *in, size_t length, unsigned char *out, const char *key)
{
	AES_KEY aes_key;
	AES_set_encrypt_key((unsigned char*)key, 128, &aes_key);

	size_t round = length / AES_BLOCK_SIZE;

	for (size_t i = 0; i < round; i++)
	{
		AES_encrypt(in, out, &aes_key);
		in += AES_BLOCK_SIZE;
		out += AES_BLOCK_SIZE;
	}
	
	size_t left = length % AES_BLOCK_SIZE;
	if (left)
	{
		unsigned char buf[AES_BLOCK_SIZE] = {0};
		memcpy(buf, in, left);
		AES_encrypt(buf, out, &aes_key);
	}
}

void AES_ecb128_decrypt(const unsigned char *in, size_t length, unsigned char *out, const char *key)
{
	AES_KEY aes_key;
	AES_set_decrypt_key((unsigned char*)key, 128, &aes_key);

	size_t round = length / AES_BLOCK_SIZE;

	for (size_t i = 0; i < round; i++)
	{
		AES_decrypt(in, out, &aes_key);
		in += AES_BLOCK_SIZE;
		out += AES_BLOCK_SIZE;
	}
}

std::string AES_ecb128_encrypt(const std::string& in)
{
	std::string key = get_key();
	size_t encode_length = ((in.size() / AES_BLOCK_SIZE) + ((in.size() % AES_BLOCK_SIZE) ? 1 : 0)) * AES_BLOCK_SIZE;
	unsigned char *buffer = new unsigned char[encode_length];
	AES_ecb128_encrypt((unsigned char*)in.c_str(), in.size(), (unsigned char*)buffer, key.c_str());
	std::string str_encode = Base64Encode((char*)buffer, encode_length);
	delete[] buffer;
	return str_encode;
}

std::string AES_ecb128_decrypt(const std::string& in)
{
	std::string key = get_key();
	size_t length = 0;
	char *bin_in = NULL;
	Base64Decode(in.c_str(), bin_in, length);
	unsigned char *bin_out = new unsigned char[length + 1];
	AES_ecb128_decrypt((unsigned char*)bin_in, length, bin_out, key.c_str());
	bin_out[length] = 0;
	std::string str_decode((char*)bin_out);
	delete[] bin_in;
	delete[] bin_out;
	return str_decode;
}

std::string AES_ecb128_encrypt(const std::string& in, const std::string& key_seed)
{
	std::string key = get_key(key_seed);
	size_t encode_length = ((in.size() / AES_BLOCK_SIZE) + ((in.size() % AES_BLOCK_SIZE) ? 1 : 0)) * AES_BLOCK_SIZE;
	unsigned char *buffer = new unsigned char[encode_length];
	AES_ecb128_encrypt((unsigned char*)in.c_str(), in.size(), (unsigned char*)buffer, key.c_str());
	std::string str_encode = Base64Encode((char*)buffer, encode_length);
	delete[] buffer;
	return str_encode;
}

std::string AES_ecb128_decrypt(const std::string& in, const std::string& key_seed)
{
	std::string key = get_key(key_seed);
	size_t length = 0;
	char *bin_in = NULL;
	Base64Decode(in.c_str(), bin_in, length);
	unsigned char *bin_out = new unsigned char[length + 1];
	AES_ecb128_decrypt((unsigned char*)bin_in, length, bin_out, key.c_str());
	bin_out[length] = 0;
	std::string str_decode((char*)bin_out);
	delete[] bin_in;
	delete[] bin_out;
	return str_decode;
}

void AES_ecb128_decrypt(const char *in, size_t length, char *out, size_t& out_len, const char *key_seed)
{
	std::string key = get_key(key_seed);
	char *bin_in = NULL;
	Base64Decode(in, length, bin_in, &out_len);
	AES_ecb128_decrypt((unsigned char*)bin_in, out_len, (unsigned char*)out, key.c_str());
	delete[] bin_in;
}
