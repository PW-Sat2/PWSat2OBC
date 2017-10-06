/***************************************************************************/ /**
c  * @file
  * @brief   CubeComputer Test Program Main.
  * @author  Pieter Botma
  * @version 2.0
  ******************************************************************************/
#include "main.hpp"
#include <em_usart.h>
#include "boot.h"
#include "boot/params.hpp"
#include "includes.h"
#include "system.h"

#define COMMS_TIMEOUT 5000 // milliseconds

decltype(Bootloader) Bootloader;

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

    Bootloader.Initialize();
}

void waitForComms(uint32_t timeoutTicks_ms)
{
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

extern "C" void __libc_init_array(void);

int main(void)
{
    __libc_init_array();

    Initialize();

    boot::RequestedRunlevel = boot::Runlevel::Runlevel3;
    boot::ClearStateOnStartup = false;

    auto d = StayInBootloader();

    if (d)
    {
        BSP_UART_Puts(BSP_UART_DEBUG, "\n\nBootloader (? = help):");

        waitForComms(COMMS_TIMEOUT);
    }

    BSP_UART_Puts(BSP_UART_DEBUG, "\nTimeout exceeded - booting");
    ProceedWithBooting();
}

OBCBootloader::OBCBootloader()
    :                                                     //
      Settings(_fram),                                    //
      BootTable(_flash),                                  //
      _fram1Spi(_spi),                                    //
      _fram2Spi(_spi),                                    //
      _fram3Spi(_spi),                                    //
      _fram1(_fram1Spi),                                  //
      _fram2(_fram2Spi),                                  //
      _fram3(_fram3Spi),                                  //
      _fram(_error_counter, {&_fram1, &_fram2, &_fram3}), //
      _flash(io_map::ProgramFlash::FlashBase)             //
{
}

void OBCBootloader::Initialize()
{
    this->_spi.Initialize();

    this->_fram1Spi.Initialize();
    this->_fram2Spi.Initialize();
    this->_fram3Spi.Initialize();

    this->_flash.Initialize();
    this->BootTable.Initialize();
}
