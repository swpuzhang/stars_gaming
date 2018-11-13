#ifndef BIGENDLITTLEEND_H
#define BIGENDLITTLEEND_H
#include <assert.h>
#include "Types.h"
#define Swap16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff)) 

#define Swap32(l) (((l) >> 24) | \
	(((l) & 0x00ff0000) >> 8)  | \
	(((l) & 0x0000ff00) << 8)  | \
	((l) << 24))  

#define Swap64(ll) (((ll) >> 56) | \
	(((ll) & 0x00ff000000000000) >> 40) | \
	(((ll) & 0x0000ff0000000000) >> 24) | \
	(((ll) & 0x000000ff00000000) >> 8)  | \
	(((ll) & 0x00000000ff000000) << 8)  | \
	(((ll) & 0x0000000000ff0000) << 24) | \
	(((ll) & 0x000000000000ff00) << 40) | \
	(((ll) << 56))) 

bool is_big_endian(void);

unsigned short hton16(unsigned short s);

unsigned short ntoh16(unsigned short s);

unsigned int hton32(unsigned int i);

unsigned int ntoh32(unsigned int i);

unsigned long long hton64(unsigned long long l);

unsigned long long ntoh64(unsigned long long l);

template <class T>
void read_integer(char* p, T& i);
#endif 