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
#include "boot/params.hpp"
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

void LESENSE_IRQHandler()
{
    uart.OnWakeUpInterrupt();
    System::EndSwitchingISR();
}

__attribute__((optimize("O3"))) void UART1_RX_IRQHandler()
{
    uart.OnReceived();
}

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

void Id(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    TerminalObject.Printf("I'm '%s'", __FILE__);
}

void BootParams(std::uint16_t /*argc*/, char* /*argv*/ [])
{
    TerminalObject.Printf("Slots: %.2X\n", boot::Index);
    TerminalObject.Printf("Reason: %d\n", num(boot::BootReason));
}

static const TerminalCommandDescription commands[] = {
    {"SetGroup", SetGroup}, //
    {"echo", Echo},
    {"id", Id},         //
    {"reset", Restart}, //
    {"bp", BootParams}, //
};

static void InitializeTerminal(void)
{
    TerminalObject.Initialize();
    TerminalObject.SetCommandList(gsl::span<const TerminalCommandDescription>(commands));
}

extern "C" void vApplicationIdleHook(void)
{
    EMU_EnterEM1();
}

extern "C" void vApplicationTickHook(void)
{
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

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);

    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, true);

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
