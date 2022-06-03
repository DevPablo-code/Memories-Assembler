#pragma once

#include <cstdint>

static uint8_t getNumberSize(int64_t number) 
{
	if (number >= INT8_MIN && number <= INT8_MAX) 
	{
		return 1;
	}
	if (number >= INT16_MIN && number <= INT16_MAX)
	{
		return 2;
	}
	if (number >= INT32_MIN && number <= INT32_MAX)
	{
		return 4;
	}
	if (number >= INT64_MIN && number <= INT64_MAX)
	{
		return 8;
	}
	return 0;
}