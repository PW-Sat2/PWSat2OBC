#include <cstdio>
#include <cstring>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_dbg.h>
#include <em_device.h>
#include <em_emu.h>
#include <em_gpio.h>
#include <gsl/span>
#include <em_system.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include "SwoEndpoint/SwoEndpoint.h"
#include "adcs/AdcsExperimental.hpp"
#include "base/ecc.h"
#include "base/os.h"
#include "dmadrv.h"
#include "eps/eps.h"
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
#include "storage/nand.h"
#include "storage/nand_driver.h"
#include "storage/storage.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

using services::time::TimeProvider;
using namespace std::chrono_literals;

static constexpr std::uint32_t PersistentStateBaseAddress = 4;

OBC Main;
mission::ObcMission Mission(std::tie(Main.timeProvider, Main.rtc),
    Main.antennaDriver,
    false,
    Main.adcs.GetAdcsController(),
    Main.Experiments.ExperimentsController,
    Main.Communication.CommDriver);

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

extern "C" void vApplicationIdleHook(void)
{
    EMU_EnterEM1();
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

static void BlinkLed0(void* param)
{
    UNREFERENCED_PARAMETER(param);

    while (1)
    {
        Main.Hardware.Pins.Led0.Toggle();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void SmartWaitTask(void* param)
{
    UNREFERENCED_PARAMETER(param);

    LOG(LOG_LEVEL_DEBUG, "Wait start");

    while (1)
    {
        Main.timeProvider.LongDelay(10min);
        LOG(LOG_LEVEL_DEBUG, "After wait");
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

static void ProcessState(OBC* obc)
{
    if (obc->Hardware.Pins.SysClear.Input() == false)
    {
        LOG(LOG_LEVEL_WARNING, "Clearing state on startup");

        if (OS_RESULT_FAILED(obc->Storage.ClearStorage()))
        {
            LOG(LOG_LEVEL_ERROR, "Clearing state failed");
        }

        LOG(LOG_LEVEL_INFO, "All files removed");
    }
    else
    {
        obc::ReadPersistentState(Mission.GetState().PersistentState, PersistentStateBaseAddress, obc->persistentStorage);
    }
}

static void SetupAntennas(void)
{
    AntennaMiniportInitialize(&Main.antennaMiniport);
    AntennaDriverInitialize(&Main.antennaDriver, &Main.antennaMiniport, &Main.Hardware.I2C.Buses.Bus, &Main.Hardware.I2C.Buses.Payload);
}

static void ObcInitTask(void* param)
{
    LOG(LOG_LEVEL_INFO, "Starting initialization task... ");

    auto obc = static_cast<OBC*>(param);

    if (OS_RESULT_FAILED(obc->PostStartInitialization()))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize hardware after start. ");
    }

    ProcessState(obc);

    obc->fs.MakeDirectory("/a");

    if (!obc->timeProvider.Initialize(nullptr, nullptr))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize persistent timer. ");
    }

    if (OS_RESULT_FAILED(Main.antennaDriver.HardReset(&Main.antennaDriver)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to reset both antenna controllers. ");
    }

    if (!obc->Communication.CommDriver.Restart())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to restart comm");
    }

    Mission.Initialize();

    obc->Hardware.Burtc.Start();

    System::CreateTask(SmartWaitTask, "SmartWait", 512, NULL, TaskPriority::P1, NULL);

    LOG(LOG_LEVEL_INFO, "Intialized");
    Main.initialized = true;

    System::SuspendTask(NULL);
}

void SetupHardware(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
}

extern "C" void __libc_init_array(void);

int main(void)
{
    memset(&Main, 0, sizeof(Main));

    __libc_init_array();

    CHIP_Init();

    SetupHardware();

    SwoEnable();

    LogInit(LOG_LEVEL_DEBUG);
    InitSwoEndpoint();

    DMADRV_Init();

    LeuartLineIOInit(&Main.IO);

    EpsInit(&Main.Hardware.I2C.Fallback);

    EPSPowerControlInitialize(&Main.PowerControlInterface);

    Main.Initialize();

    InitializeTerminal();

    SwoPutsOnChannel(0, "Hello I'm PW-SAT2 OBC\n");

    SetupAntennas();

    Main.Hardware.Pins.Led0.High();
    Main.Hardware.Pins.Led1.High();

    System::CreateTask(BlinkLed0, "Blink0", 512, NULL, TaskPriority::P1, NULL);
    System::CreateTask(ObcInitTask, "Init", 2_KB, &Main, TaskPriority::Highest, &Main.initTask);

    System::RunScheduler();

    Main.Hardware.Pins.Led0.Toggle();

    return 0;
}
