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

template <int Tag, typename Head, typename... Pins> static bool CanTouch(GPIO_Port_TypeDef port, std::uint8_t pin)
{
    if (port == Head::Port && pin == Head::PinNumber)
    {
        return false;
    }

    return CanTouch<Tag, Pins...>(port, pin);
}

template <int Tag> static bool CanTouch(GPIO_Port_TypeDef port, std::uint8_t pin)
{
    return true;
}

template <typename... LeavePins> void InitializeAllPins(GPIO_Port_TypeDef port, std::uint8_t pinCount, io_map::PinContainer<LeavePins...>)
{
    static_assert(sizeof...(LeavePins) == 11, "Invalid number of leave pins");

    for (std::uint8_t pin = 0; pin < pinCount; pin++)
    {
        if (!CanTouch<0, LeavePins...>(port, pin))
        {
            /*
             * Let me tell you the story...
             *
             * # Chapter 1
             *
             * On dark, rainy evening in Brite room three developers and one electronics guy
             * were sitting in lab in CBK peacefully testing various scenarios for OBC. Suddenly,
             * when no one expected, the Dragon in bootloader appeared. Health check was passing
             * regardless of serious mismatch of CRCs (developers expected fails). Developers rushed
             * to computers to debug the issue like there is no tomorrow. But no one of them expected
             * what was comming for them
             *
             * # Chapter 2
             * Developers started by flashing new version of bootloader to FlightModel and Stanger Things
             * happened. The Great Dragon, much greater than previous one appear surrounded by flames and
             * eternal screams of mortals. Bootloader kept resetting, every second the Blue LED of Live was
             * blinking with dim light predicting inevitable...
             *
             * # Chapter 3
             * The brave electronic had that legendary moment of brightness. Together with Mister Piotr they
             * started checking the obvious things - voltage, over-current protection and RESET signal. They saw
             * what they predicted - repeating every 1 second of RESET signal. In the meantime Tomek The Developer
             * started looking at the changes between bootloader on FM and newest. There were only one significant
             * change - initialization of all pins at the very begining of boot process.
             *
             * # Chapter 4
             * It was the moment when everything started to match - external watchdog was activated by pin being disabled
             * and kept resetting OBC after a second.
             *
             * # Epilogue
             * DON"T MESS WITH PINS THAT ARE CONNECTED TO SOMETHING YOU DON'T CONTROL
             *
             */
            continue;
        }

        GPIO_PinModeSet(port, pin, gpioModeDisabled, 1);
    }
}

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

    CMU_ClockEnable(cmuClock_GPIO, true);

    using LeavePins = io_map::Flatten<io_map::DontTouchPins>::Result;

    InitializeAllPins(gpioPortA, _GPIO_PORT_A_PIN_COUNT, LeavePins());
    InitializeAllPins(gpioPortB, _GPIO_PORT_B_PIN_COUNT, LeavePins());
    InitializeAllPins(gpioPortC, _GPIO_PORT_C_PIN_COUNT, LeavePins());
    InitializeAllPins(gpioPortD, _GPIO_PORT_D_PIN_COUNT, LeavePins());
    InitializeAllPins(gpioPortE, _GPIO_PORT_E_PIN_COUNT, LeavePins());
    InitializeAllPins(gpioPortF, _GPIO_PORT_F_PIN_COUNT, LeavePins());

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
