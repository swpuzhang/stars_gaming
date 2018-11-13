#ifndef _MD5_H_
#define _MD5_H_

#include <stdio.h>
#include "BaseToolConfig.h"

/************************************************
* MD5 º”√‹À„∑®
*
*************************************************/


int  MD5_16(const char* const data, size_t data_len, char out_put[16]);

int  MD5_32(const char* const data, size_t data_len, char out_put[32]);

std::string  MD5_32(const char* const data, size_t data_len);

int  MD5_EncodeFile(FILE *pFile,void *pOut);

bool  Md5_Check(const char pSrc[16],const char pDst[16]);


#endif //_MD5_H_