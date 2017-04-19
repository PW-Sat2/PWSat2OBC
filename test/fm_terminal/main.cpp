#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include "SwoEndpoint/SwoEndpoint.h"
#include "base/os.h"
#include "dmadrv.h"
#include "gpio/gpio.h"
#include "io_map.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal/terminal.h"
#include "uart/uart.h"

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

OSEventGroupHandle GlobalEventGroup;

drivers::uart::UART uart;
Terminal TerminalObject(uart.GetLineIO());

void SetGroup(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    System::EventGroupSetBits(GlobalEventGroup, 0x80);
}

void Echo(uint16_t argc, char* argv[])
{
    for (std::uint16_t i = 0; i < argc; ++i)
    {
        TerminalObject.Printf("argument: '%d', %s\n", i, argv[i]);
    }
}

void Restart(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    NVIC_SystemReset();
}

static const TerminalCommandDescription commands[] = {
    {"SetGroup", SetGroup}, //
    {"echo", Echo},
    {"reset", Restart} //
};

static void InitializeTerminal(void)
{
    TerminalObject.Initialize();
    TerminalObject.SetCommandList(gsl::span<const TerminalCommandDescription>(commands));
}

extern "C" void vApplicationStackOverflowHook(xTaskHandle* pxTask, signed char* pcTaskName)
{
    UNREFERENCED_PARAMETER(pxTask);
    UNREFERENCED_PARAMETER(pcTaskName);
}

extern "C" void vApplicationIdleHook(void)
{
    EMU_EnterEM1();
}

static void InitSwoEndpoint(void)
{
    void* swoEndpointHandle = SwoEndpointInit();
    const bool result = LogAddEndpoint(SwoGetEndpoint(swoEndpointHandle), swoEndpointHandle, LOG_LEVEL_TRACE);
    if (!result)
    {
        SwoPutsOnChannel(0, "Unable to attach swo endpoint to logger. ");
    }
}

using namespace drivers::gpio;

InputPin<io_map::UART::RX> RXPin;
OutputPin<io_map::UART::TX> TXPin;

extern "C" void __libc_init_array(void);

int main(void)
{
    SCB->VTOR = 0x80000;

    CHIP_Init();

    __libc_init_array();

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFRCO);

    SwoEnable();

    LogInit(LOG_LEVEL_DEBUG);
    InitSwoEndpoint();

    DMADRV_Init();

    RXPin.Initialize();
    TXPin.Initialize();

    uart.Initialize();

    GlobalEventGroup = System::CreateEventGroup();

    InitializeTerminal();

    SwoPutsOnChannel(0, "Ready\n");

    System::RunScheduler();

    return 0;
}
