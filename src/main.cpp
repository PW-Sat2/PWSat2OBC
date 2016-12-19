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
#include "adcs/adcs.h"
#include "base/ecc.h"
#include "base/os.h"
#include "dmadrv.h"
#include "eps/eps.h"
#include "fs/fs.h"
#include "i2c/i2c.h"
#include "io_map.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "mission.h"
#include "obc.h"
#include "storage/nand.h"
#include "storage/nand_driver.h"
#include "storage/storage.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal.h"

#include "gpio/gpio.h"
#include "leuart/leuart.h"
#include "power_eps/power_eps.h"
#include "uart/Uart.h"

using devices::comm::CommObject;
using devices::comm::CommFrame;

using services::time::TimeProvider;
using namespace std::chrono_literals;
using devices::comm::CommObject;
using devices::comm::CommFrame;
using gsl::span;
using namespace drivers::uart;

OBC Main;
mission::ObcMission Mission(Main.timeProvider, Main.antennaDriver, false);

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

extern "C" void vApplicationStackOverflowHook(xTaskHandle* pxTask, signed char* pcTaskName)
{
    UNREFERENCED_PARAMETER(pxTask);
    UNREFERENCED_PARAMETER(pcTaskName);
}

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

static void BlinkLed0(void* param)
{
    UNREFERENCED_PARAMETER(param);

    while (1)
    {
        Main.Hardware.Pins.Led0.Toggle();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
/*
static void SmartWaitTask(void* param)
{
    UNREFERENCED_PARAMETER(param);

    LOG(LOG_LEVEL_DEBUG, "Wait start");

    while (1)
    {
        Main.timeProvider.LongDelay(10min);
        LOG(LOG_LEVEL_DEBUG, "After wait");
    }
}*/

static void InitSwoEndpoint(void)
{
    void* swoEndpointHandle = SwoEndpointInit();
    const bool result = LogAddEndpoint(SwoGetEndpoint(swoEndpointHandle), swoEndpointHandle, LOG_LEVEL_TRACE);
    if (!result)
    {
        SwoPutsOnChannel(0, "Unable to attach swo endpoint to logger. ");
    }
}

static void ClearState(OBC* obc)
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
}

static void SetupAntennas(void)
{
    AntennaMiniportInitialize(&Main.antennaMiniport);
    AntennaDriverInitialize(&Main.antennaDriver, &Main.antennaMiniport, &Main.Hardware.I2C.Buses.Bus, &Main.Hardware.I2C.Buses.Payload);
}

static void ObcInitTask(void* param)
{
    auto obc = static_cast<OBC*>(param);

    obc->PostStartInitialization();

    ClearState(obc);

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

    InitializeADCS(&obc->adcs);

    Mission.Initialize();

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


void UartTask(void* param)
{
	const char* testByte="lamakota";
	UNREFERENCED_PARAMETER(param);

	Uart_Init init;
	init.uart = USART1;
	init.baudRate=9600;
	init.parity=usartNoParity;
	init.portLocation=_USART_ROUTE_LOCATION_LOC1;
	init.baudRate       = 9600;
	init.oversampling   = usartOVS16;
	init.dataBits       = usartDatabits8;
	init.parity         = usartNoParity;
	init.stopBits       = usartStopbits1;


	Uart uart(init);
	uart.Initialize();
	while (1) {
	uart.Write(gsl::span<const uint8_t>(reinterpret_cast<const uint8_t*>(testByte), 8));
	System::SleepTask(10);
	}

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

    //System::CreateTask(ADXRS, "ADXRS", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
    System::CreateTask(UartTask, "uart", 512, NULL, TaskPriority::P1, NULL);
    System::CreateTask(ObcInitTask, "Init", 512, &Main, TaskPriority::Highest, &Main.initTask);

    System::RunScheduler();

    Main.Hardware.Pins.Led0.Toggle();

    return 0;
}
