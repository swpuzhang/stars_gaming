#include "Base64.h"
#include <string.h>
#include <assert.h>

const char base64o[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char base64i[128] = {
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
-1, 26, 27, 28, 29, 30,	31, 32, 33, 34, 35, 36, 37, 38, 39,	40,
41, 42, 43, 44, 45, 46,	47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

const char base64o_ex[65] = "Tiger_xu1984206bz#5%3$WwpIQ7*&[,]mNoYEs@!XUHBaZCqRtOcD<fGF+>.hjJ";
const char base64i_ex[128] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, 40, -1, 17, 21, 19, 29, -1, 
	-1, -1, 28, 58, 31, -1, 60, -1, 13, 8, 
	12, 20, 11, 18, 14, 27, 10, 9, -1, -1, 
	54, -1, 59, -1, 39, -1, 44, 47, 53, 37, 
	57, 56, 43, 25, 63, -1, -1, -1, 34, 51, 
	-1, 26, 49, -1, 0, 42, -1, 22, 41, 36, 
	46, 30, -1, 32, -1, 5, -1, 45, 15, 52, 
	-1, 3, 55, 2, 61, 1, 62, -1, -1, 33, 
	-1, 35, 24, 48, 4, 38, 50, 7, -1, 23, 
	6, -1, 16, -1, -1, -1, -1, -1
};

const char base64EndChar = '=';


std::string Base64Encode(const char* s, int n, int chSet)
{
	const char *local_base64o = NULL;
	char local_end = '=';
	switch(chSet)
	{
	case BASE64_CH_SECRET:
		{
			local_base64o = base64o_ex;
			local_end = '#';
			break;
		}
	default:
		{
			local_base64o = base64o;
			break;
		}
	}
	std::string strOutput;
	int i = 0;
	unsigned long m = 0;
	char d[5] = {0};
	for (; n > 2; n -= 3, s += 3)
	{ 
		m = s[0];
		m = (m << 8) | s[1];
		m = (m << 8) | s[2];
		for (i = 4; i > 0; m >>= 6)
		{
			d[--i] = local_base64o[m & 0x3F];
		}
		strOutput += d;
	}

	if (n > 0)
	{
		m = 0;
		for (i = 0; i < n; i++)
		{
			m = (m << 8) | *s++;
		}
		for (; i < 3; i++)
		{
			m <<= 8;
		}
		for (i++; i > 0; m >>= 6)
		{
			d[--i] = local_base64o[m & 0x3F];
		}
		for (i = 3; i > n; i--)
		{
			d[i] = local_end;
		}
		strOutput += d;
	}
	return strOutput;
}

int Base64Decode(const std::string& strEn, char *&buf, int &length, int chSet)
{
	const char *local_base64i = NULL;
	char local_end = '=';
	switch(chSet)
	{
	case BASE64_CH_SECRET:
		{
			local_base64i = base64i_ex;
			local_end = '#';
			break;
		}
	default:
		{
			local_base64i = base64i;
			break;
		}
	}
	
	int iStrLen = (int)strEn.size();
	int iBuffLen = (int)(iStrLen / 4 ) * 3 + 1;
	if (iBuffLen <= 1)
	{
		return -1;
	}

	buf = new char[iBuffLen];
	length = 0;
	char c1, c2, c3, c4;
	for (int i = 0; i < iStrLen; )
	{
		// c1
		do 
		{
			c1 = local_base64i[strEn.at(i++) & 0xFF];
		} while (i < iStrLen && c1 == -1);
		if (-1 == c1)
		{
			length = 0;
			break;
		}
		// c2
		do 
		{
			c2 = local_base64i[strEn.at(i++) & 0xFF];
		} while (i < iStrLen && c1 == -1);
		if (-1 == c2)
		{
			length = 0;
			break;
		}
		buf[length++] = ((c1 << 2) | ((c2 & 0x30) >> 4));

		/* c3 */
		do
		{
			c3 = strEn.at(i++) & 0xFF;
			if(c3 == local_end)
				return 0;
			c3 = local_base64i[(int)c3];
		} while(i < iStrLen && c3 == -1);
		if (-1 == c3)
		{
			length = 0;
			break;
		}
		buf[length++] = ((c2 & 0xF) << 4) | ((c3 & 0x3C) >> 2);

		/* c4 */
		do
		{
			c4 = strEn.at(i++) & 0xFF;
			if(c4 == local_end)
				return 0;
			c4 = local_base64i[(int)c4];
		} while(i < iStrLen && c4 == -1);
		if (-1 == c4)
		{
			length = 0;
			break;
		}
		buf[length++] = ((c3 & 0x03) << 6) | c4;
	}

	if (0 == length)
	{
		delete[] buf;
		buf = NULL;
		return -1;
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//////////////////   base64 \B1\E0\C2\EB/\BD\E2\C2\EB   ////////////////////////////////////

const int  BASE64_MAXLINE = 76;   
const char EOL[] = "\r\n";   
const char BASE64_TAB[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"   
	"abcdefghijklmnopqrstuvwxyz0123456789+/";   

// 3\D7ֽ\DAһ\B8\F6\B5\A5Ԫ\A3\AC\B6Ե\A5Ԫ\BD\F8\D0б\E0\C2\EB
inline void Unit_Code(const char *pSrc,char *pDes)
{
	unsigned char *pCh = (unsigned char*)pSrc;
	*pDes++ = BASE64_TAB[pCh[0] >> 2];
	*pDes++ = BASE64_TAB[((pCh[0] << 4) & 0x3F) | (pCh[1] >> 4)];
	*pDes++ = BASE64_TAB[((pCh[1] << 2) & 0x3F) | (pCh[2] >> 6)];
	*pDes = BASE64_TAB[pCh[2] & 0x3F];
}

// \B6\D4\CC\EE\B3\E4\D7ֽڱ\E0\C2\EB
inline void Fill_Code(const char *pSrc,char *pDes,size_t left_len)
{
	size_t fill = 3 - left_len;
	assert(fill == 1 || fill == 2);
	unsigned char *pCh = (unsigned char*)pSrc;
	unsigned char Ch_fill[3] = {0};

	if (fill == 1)
	{
		Ch_fill[0] = *pCh++;
		Ch_fill[1] = *pCh;

		*pDes++ = BASE64_TAB[Ch_fill[0] >> 2];
		*pDes++ = BASE64_TAB[((Ch_fill[0] << 4) & 0x3F) | (Ch_fill[1] >> 4)];
		*pDes++ = BASE64_TAB[((Ch_fill[1] << 2) & 0x3F) | (Ch_fill[2] >> 6)];
		*pDes = '=';
	}
	else
	{
		Ch_fill[0] = *pCh;
		*pDes++ = BASE64_TAB[Ch_fill[0] >> 2];
		*pDes++ = BASE64_TAB[((Ch_fill[0] << 4) & 0x3F) | (Ch_fill[1] >> 4)];
		*pDes++ = '=';
		*pDes = '=';
	}
}


// BASE64\D7ַ\FB\B1\ED\D6е\C4λ\D6\C3
inline unsigned char Base64ChPos(char ch)
{
	const char *pos = strchr(BASE64_TAB,ch);
	return (unsigned char)(pos - BASE64_TAB);
}

//  4\D7ֽ\DAһ\B8\F6\B5\A5Ԫ\A3\AC\B6Ե\A5Ԫ\BD\F8\D0н\E2\C2\EB
inline void Unit_Decode(const char *pSrc,char *pDes)
{
	unsigned char code_src[4] = {0};

	if (pSrc[3] != '=')
	{
		code_src[0] = Base64ChPos(*pSrc++);
		code_src[1] = Base64ChPos(*pSrc++);
		code_src[2] = Base64ChPos(*pSrc++);
		code_src[3] = Base64ChPos(*pSrc);

		*pDes++ = (code_src[0] << 2) | (code_src[1] >> 4);
		*pDes++ = (code_src[1] << 4) | (code_src[2] >> 2);
		*pDes++ = (code_src[2] << 6) | code_src[3];
	}
	else if (pSrc[2] == '=')
	{
		code_src[0] = Base64ChPos(*pSrc++);
		code_src[1] = Base64ChPos(*pSrc++);
		code_src[2] = 0;
		code_src[3] = 0;

		*pDes++ = (code_src[0] << 2) | (code_src[1] >> 4);
	}
	else
	{
		code_src[0] = Base64ChPos(*pSrc++);
		code_src[1] = Base64ChPos(*pSrc++);
		code_src[2] = Base64ChPos(*pSrc++);
		code_src[3] = 0;

		*pDes++ = (code_src[0] << 2) | (code_src[1] >> 4);
		*pDes++ = (code_src[1] << 4) | (code_src[2] >> 2);
	}
}

std::string Base64Encode(const char* src, size_t length)
{
	size_t encode_length = ((length / 3) + ((length % 3) ? 1 : 0)) * 4;
	char *buf = new char[encode_length + 1];
	Base64Encode(src, length, buf);
	buf[encode_length] = 0;
	std::string str_encode(buf);
	delete[] buf;
	return str_encode;
}

void Base64Encode(const char* src, size_t src_length, char* dest, size_t *encode_length)
{
	size_t left = src_length % 3;
	size_t unit_count = src_length / 3;
	if (encode_length)
	{
		*encode_length = ((src_length / 3) + ((src_length % 3) ? 1 : 0)) * 4;
	}

	for (size_t i = 0; i < unit_count; i++)
	{
		Unit_Code(src, dest);
		src += 3;
		dest += 4;
	}

	if (left == 0)
	{
		return;
	}

	// code last 3 byte
	Fill_Code(src, dest, left);
}

void Base64Decode(const char* src, size_t src_length, char* dest, size_t *decode_length)
{
	if (src_length < 4)
	{
		return;
	}

	size_t padding = 0;
	if (src[src_length - 1] == '=')
	{
		padding++;
		if (src[src_length - 2] == '=')
		{
			padding++;
		}
	}

	size_t unit_count = src_length / 4;
	for (size_t i = 0; i < unit_count; i++)
	{
		Unit_Decode(src, dest);
		src += 4;
		dest += 3;
	}

	if (decode_length)
	{
		*decode_length = unit_count * 3 - padding;
	}
}

int Base64Decode(const std::string& strEn, char *&buf, size_t &length)
{
	if (strEn.empty())
	{
		length = 0;
		return 0;
	}
	
	size_t  buf_size = (strEn.size() / 4) * 3;
	buf = new char[buf_size];
	Base64Decode(strEn.c_str(), strEn.size(), buf, &length);
	return 0;
}
