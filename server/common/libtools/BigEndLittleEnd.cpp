
#include <assert.h>
#include "Types.h"
#include "BigEndLittleEnd.h"
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

bool is_big_endian(void)
{  
	const short n = 1;  
	if(*(char *)&n)  
	{  
		return false;  
	}  
	return true;  
}  

unsigned short hton16(unsigned short s)
{
	static bool big_endian = is_big_endian();
	return big_endian ? s : Swap16(s);
}

 unsigned short ntoh16(unsigned short s)
{
	static bool big_endian = is_big_endian();
	return big_endian ? s : Swap16(s);
}

 unsigned int hton32(unsigned int i)
{
	//return ::htonl(i);
	static bool big_endian = is_big_endian();
	return big_endian ? i : Swap32(i);
}

 unsigned int ntoh32(unsigned int i)
{
	//return ::ntohl(i);
	static bool big_endian = is_big_endian();
	return big_endian ? i : Swap32(i);
}

 unsigned long long hton64(unsigned long long l)
{
	static bool big_endian = is_big_endian();
	return big_endian ? l : Swap64(l);
}

 unsigned long long ntoh64(unsigned long long l)
{
	static bool big_endian = is_big_endian();
	return big_endian ? l : Swap64(l);
}

template <class T>
 void read_integer(char* p, T& i)
{
	assert(p);
	switch (sizeof(T))
	{
	case 2:
		{
			i = (T)ntoh16((TY_UINT32)*p);
			break;
		}
	case 4:
		{
			i = (T)ntoh32((TY_UINT32)*p);
			break;
		}
	case 8:
		{
			i = (T)ntoh64(*p);
			break;
		}
	default:
		{
			i = *p;
			break;
		}
	}
}