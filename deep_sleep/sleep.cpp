#include <cstdint>
#include <em_chip.h>
#include <chrono>

using namespace std::chrono_literals;

void DWT_Init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

void DWT_Delay(std::chrono::microseconds us) // microseconds
{
    DWT->CYCCNT = 0;
    uint32_t targetTick = us.count() * (SystemCoreClock/1000000);

    while (DWT->CYCCNT < targetTick);
}

void Sleep(std::chrono::milliseconds delay)
{
    DWT_Delay(std::chrono::duration_cast<std::chrono::microseconds>(delay));
}