/***************************************************************************/ /**
  * @file
  * @brief   CubeComputer Test Program Main.
  * @author  Pieter Botma
  * @version 2.0
  ******************************************************************************/
#include <em_usart.h>
#include "boot.h"
#include "boot/params.hpp"
#include "includes.h"
#include "system.h"

#define COMMS_TIMEOUT 5000 // milliseconds

void waitForComms(uint32_t timeoutTicks_ms);

static void Initialize()
{
    CHIP_Init();

    // set up general clocks
    CMU_OscillatorEnable(cmuOsc_HFRCO, true, true);
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
    CMU_HFRCOBandSet(cmuHFRCOBand_14MHz);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);

    if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) // Setup SysTick Timer for 1 msec interrupts
    {
        while (1)
            ;
    }

    BSP_DMA_Init();
    COMMS_Init();
    BSP_EBI_Init();
    BSP_EBI_disableSRAM(bspEbiSram1);
    BSP_EBI_disableSRAM(bspEbiSram2);
    Delay(5);
    BSP_EBI_enableSRAM(bspEbiSram1);
    BSP_EBI_enableSRAM(bspEbiSram2);
}

void waitForComms(uint32_t timeoutTicks_ms)
{
    BSP_UART_txByte(BSP_UART_DEBUG, '#');
    while (true)
    {
        COMMS_processMsg();

        EMU_EnterEM1();
    }
}

static bool StayInBootloader()
{
    USART_IntDisable(BSP_UART_DEBUG, USART_IF_RXDATAV);
    msTicks = 0;

    BSP_UART_txByte(BSP_UART_DEBUG, '&');

    do
    {
        if (has_flag(USART_StatusGet(BSP_UART_DEBUG), USART_STATUS_RXDATAV))
        {
            auto c = USART_RxDataGet(BSP_UART_DEBUG);

            if (c == 'S')
            {
                BSP_UART_txByte(BSP_UART_DEBUG, 'O');
                return true;
            }
        }

        EMU_EnterEM1();
    } while (msTicks < 5000);

    return false;
}

int main(void)
{
    Initialize();

    boot::RequestedRunlevel = boot::Runlevel::Runlevel2;

    auto d = StayInBootloader();

    if (d)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBootloader (? = help):");

        waitForComms(COMMS_TIMEOUT);
    }

    ProceedWithBooting();
}
