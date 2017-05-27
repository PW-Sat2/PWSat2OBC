#include <cstdio>
#include <cstring>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_dbg.h>
#include <em_device.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <em_timer.h>
#include <gsl/span>
#include <em_system.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include "SwoEndpoint/SwoEndpoint.h"
#include "adcs/AdcsExperimental.hpp"
#include "base/ecc.h"
#include "base/os.h"
#include "boot/params.hpp"
#include "dmadrv.h"
#include "efm_support/clock.h"
#include "fs/fs.h"
#include "gpio/gpio.h"
#include "i2c/i2c.h"
#include "io_map.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "mission.h"
#include "obc.h"
#include "obc/ObcState.hpp"
#include "power_eps/power_eps.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"
#include "watchdog/watchdog.hpp"

using services::time::TimeProvider;
using namespace std::chrono_literals;

OBC Main;
mission::ObcMission Mission(std::tie(Main.timeProvider, Main.Hardware.rtc),
    Main.Hardware.antennaDriver,
    Main.Communication.CommDriver,
    std::tuple<bool, services::power::IPowerControl&>(false, Main.PowerControlInterface),
    Main.adcs.GetAdcsController(),
    Main.Experiments.ExperimentsController,
    Main.Communication.CommDriver,
    std::tie(Main.Hardware.PersistentStorage, PersistentStateBaseAddress),
    Main.fs);

telemetry::ObcTelemetryAcquisition TelemetryAcquisition(Main.Communication.CommDriver,
    std::tuple<services::fs::IFileSystem&, mission::TelemetryConfiguration>(
        Main.fs, mission::TelemetryConfiguration{"/telemetry.current", "/telemetry.previous", 512_KB}));

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

extern "C" void vApplicationIdleHook(void)
{
    EMU_EnterEM1();
}

extern "C" void vApplicationTickHook(void)
{
    drivers::watchdog::InternalWatchdog::Kick();
}

void I2C0_IRQHandler(void)
{
    Main.Hardware.I2C.Peripherals[0].Driver.IRQHandler();
}

void I2C1_IRQHandler(void)
{
    Main.Hardware.I2C.Peripherals[1].Driver.IRQHandler();
}

void BURTC_IRQHandler(void)
{
    Main.Hardware.Burtc.IRQHandler();
}

void LESENSE_IRQHandler()
{
    Main.Hardware.UARTDriver.OnWakeUpInterrupt();
    System::EndSwitchingISR();
}

void TIMER0_IRQHandler()
{
    Scrubber::Scrub();
    TIMER_IntClear(io_map::RAMScrubbing::TimerHW, TIMER_IFC_OF);
}

__attribute__((optimize("O3"))) void UART1_RX_IRQHandler()
{
#define DEBUG_UART
#ifdef DEBUG_UART
    GPIO->P[gpioPortC].DOUTCLR = 1 << 11;
#endif
#undef DEBUG_UART

    Main.Hardware.UARTDriver.OnReceived();
}

static void BlinkLed0(void* param)
{
    UNREFERENCED_PARAMETER(param);

    while (1)
    {
        Main.Hardware.Pins.Led0.Toggle();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
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

static void ObcInitTask(void* param)
{
    drivers::watchdog::InternalWatchdog::Enable();

    LOG(LOG_LEVEL_INFO, "Starting initialization task... ");
    LOGF(LOG_LEVEL_INFO, "Requested runlevel %d", num(boot::RequestedRunlevel));

    auto obc = static_cast<OBC*>(param);

    if (OS_RESULT_FAILED(obc->InitializeRunlevel1()))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize OBC. ");
    }

    System::SuspendTask(NULL);
}

static void SetupRAMScrubbing()
{
    CMU_ClockEnable(efm::Clock(io_map::RAMScrubbing::TimerHW), true);

    TIMER_Init_TypeDef init = TIMER_INIT_DEFAULT;
    init.count2x = false;
    init.enable = false;
    init.mode = timerModeUp;
    init.prescale = io_map::RAMScrubbing::Prescaler;

    TIMER_Init(TIMER0, &init);

    TIMER_TopSet(TIMER0, io_map::RAMScrubbing::TimerTop);

    TIMER_IntEnable(io_map::RAMScrubbing::TimerHW, TIMER_IEN_OF);

    NVIC_EnableIRQ(io_map::RAMScrubbing::IRQ);

    TIMER_Enable(io_map::RAMScrubbing::TimerHW, true);
}

void SetupHardware(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);

#ifndef SLOWER_CLOCK
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, true);
#endif

    SetupRAMScrubbing();
}

extern "C" void __libc_init_array(void);

int main(void)
{
    memset(&Main, 0, sizeof(Main));

    __libc_init_array();

    CHIP_Init();

    drivers::watchdog::InternalWatchdog::Disable();

    SetupHardware();

    SwoEnable();

    LogInit(LOG_LEVEL_DEBUG);
    InitSwoEndpoint();

    DMADRV_Init();

#ifdef USE_LEUART
    LeuartLineIOInit(&Main.IO);
#endif

    if (boot::MagicNumber != boot::BootloaderMagicNumber)
    {
        LOGF(LOG_LEVEL_WARNING,
            "No boot information from bootloader (expected: 0x%lX, got: 0x%lX)",
            boot::BootloaderMagicNumber,
            boot::MagicNumber);

        boot::RequestedRunlevel = boot::Runlevel::Runlevel2;
    }
    else
    {
        LOG(LOG_LEVEL_DEBUG, "Received boot information from bootloader");
    }

    Main.Initialize();

    SwoPutsOnChannel(0, "Hello I'm PW-SAT2 OBC\n");

    Main.Hardware.Pins.Led0.High();
    Main.Hardware.Pins.Led1.High();

    System::CreateTask(BlinkLed0, "Blink0", 512, NULL, TaskPriority::P1, NULL);
    System::CreateTask(ObcInitTask, "Init", 8_KB, &Main, TaskPriority::P14, &Main.initTask);

    System::RunScheduler();

    Main.Hardware.Pins.Led0.Toggle();

    return 0;
}
