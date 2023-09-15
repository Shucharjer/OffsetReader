#include "Byte.h"

#include <iostream>

char Byte::ToByte(const char* buf)
{
	// when strtol() meeting space, it will stop and return the value
	return strtol(buf, NULL, 16);
}

void Byte::ValueToBYTES(const char* src, char* dst, pAddr& length)
{
	char* record = dst;
	length = 0;
	while (true)
	{
		*dst = ToByte(src);
		dst++;
		length++;
		while (*src != ' ' && *src != '\0')
		{
			src++;
		}
		if (*src == ' ') src++;
		if (*src == '\0') break;
	}
	dst = record;
}