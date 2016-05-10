#include <stdint.h>

static uint32_t time = 0;

uint32_t CurrentTime(void)
{
    return time;
}

void JumpToTime(uint32_t targetTime)
{
    time = targetTime;
}
