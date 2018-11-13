// basetool.cpp : 定义 DLL 应用程序的入口点。
//

#include "BaseTool.h"
#include <memory.h>
const int TEMP_BUF_SIZE = 1024 * 128 ;

std::string bin2hex(const unsigned char *p, size_t len)
{
	const char	*hex = "0123456789abcdef";
	char *buf = new char[2*len+1];
	char *to = buf;
	for (;len--; p++) {
		*to++ = hex[p[0] >> 4];
		*to++ = hex[p[0] & 0x0f];
	}
	*to++ = 0;
	std::string ret = buf;
	delete[] buf;
	return ret;
}

void bin2hex(char *to, const unsigned char *p, size_t len)
{
	const char	*hex = "0123456789abcdef";

	for (;len--; p++) {
		*to++ = hex[p[0] >> 4];
		*to++ = hex[p[0] & 0x0f];
	}
}


void hex2bin(unsigned char *to, const char *p)
{
	while (*p)
	{
		*to = *p > 'a' ? (*p - 'a' + 10) : (*p - '\0');
		*to <<= 4;
		p++;
		*to |= *p > 'a' ? (*p - 'a' + 10) : (*p - '\0');
	}
}

static unsigned char SimpleEncShift(unsigned char* srcByte)
{
	return  (*srcByte >> 6) | (*srcByte << 2);
}

static unsigned char SimpleDecShift(unsigned char* srcByte)
{
	return (*srcByte << 6) | (*srcByte >> 2);
}


void SimpLeSiftencode(const char* src, int srcLen, int shiftBit, char* des)
{
	memcpy(des, src, srcLen);
	int rightShift = srcLen - shiftBit;
	for (int i = 0; i < shiftBit; ++i)
	{
		des[i + rightShift] = src[i];
		des[i] = src[i + rightShift];
	}
	des[0] = SimpleEncShift((unsigned char*)des);
	des[srcLen - 1] = SimpleEncShift((unsigned char*)(des + srcLen - 1));
}

void SimpLeSiftdecode(const char* src, int srcLen, int shiftBit, char* des)
{
	char tempBuf[TEMP_BUF_SIZE] = { 0 };
	memcpy(tempBuf, src, srcLen);
	tempBuf[0] = SimpleDecShift((unsigned char*)tempBuf);
	tempBuf[srcLen - 1] = SimpleDecShift((unsigned char*)(tempBuf + srcLen - 1));
	memcpy(des, tempBuf, srcLen);
	int leftShift = srcLen - shiftBit;
	for (int i = 0; i < shiftBit; ++i)
	{
		des[i + leftShift] = tempBuf[i];
		des[i] = tempBuf[i + leftShift];
	}
}