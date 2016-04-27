#include <stdint.h>

static uint32_t time = 0;

uint32_t currentTime(void)
{
	return time;
}

void jumpToTime(uint32_t targetTime)
{
	time = targetTime;
}
