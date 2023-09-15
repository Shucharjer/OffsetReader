#pragma once

#include <iostream>

#ifdef _WIN64
#define pAddr unsigned __int64
#else
#define pAddr unsigned __int32
#endif

namespace Byte
{
	// transform value in character into value in memory
	char ToByte(const char* buf);
	// transform value in string into value in memory
	void ValueToBYTES(const char* src, char* dst, pAddr& length);
}