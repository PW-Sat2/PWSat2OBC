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
#include "comm/comm.h"
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

#include <spidrv.h>
#include "adxrs453/adxrs453.h"

#include "leuart/leuart.h"
#include "power_eps/power_eps.h"

using namespace std;
using devices::comm::CommObject;
using devices::comm::CommFrame;

OBC Main;
MissionState Mission;

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
    I2CIRQHandler(&Main.I2CBuses[0].Bus);
}

void I2C1_IRQHandler(void)
{
    I2CIRQHandler(&Main.I2CBuses[1].Bus);
}

static void BlinkLed0(void* param)
{
    UNREFERENCED_PARAMETER(param);

    while (1)
    {
        GPIO_PinOutToggle(LED_PORT, LED0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void SmartWaitTask(void* param)
{
    UNREFERENCED_PARAMETER(param);

    LOG(LOG_LEVEL_DEBUG, "Wait start");

    while (1)
    {
        TimeLongDelay(&Main.timeProvider, TimeSpanFromMinutes(10));
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

static bool FSInit(FileSystem* fs, struct yaffs_dev* rootDevice, YaffsNANDDriver* rootDeviceDriver)
{
    memset(rootDevice, 0, sizeof(*rootDevice));
    rootDeviceDriver->geometry.pageSize = 512;
    rootDeviceDriver->geometry.spareAreaPerPage = 0;
    rootDeviceDriver->geometry.pagesPerBlock = 32;
    rootDeviceDriver->geometry.pagesPerChunk = 1;

    NANDCalculateGeometry(&rootDeviceDriver->geometry);

    BuildNANDInterface(&rootDeviceDriver->flash);

    SetupYaffsNANDDriver(rootDevice, rootDeviceDriver);

    rootDevice->param.name = "/";
    rootDevice->param.inband_tags = true;
    rootDevice->param.is_yaffs2 = true;
    rootDevice->param.total_bytes_per_chunk = rootDeviceDriver->geometry.chunkSize;
    rootDevice->param.chunks_per_block = rootDeviceDriver->geometry.chunksPerBlock;
    rootDevice->param.spare_bytes_per_chunk = 0;
    rootDevice->param.start_block = 1;
    rootDevice->param.n_reserved_blocks = 3;
    rootDevice->param.no_tags_ecc = true;
    rootDevice->param.always_check_erased = true;

    rootDevice->param.end_block =
        1 * 1024 * 1024 / rootDeviceDriver->geometry.blockSize - rootDevice->param.start_block - rootDevice->param.n_reserved_blocks;

    return FileSystemInitialize(fs, rootDevice);
}

static void ClearState(OBC* obc)
{
    GPIO_PinModeSet(SYS_CLEAR_PORT, SYS_CLEAR_PIN, gpioModeInputPull, 1);

    if (GPIO_PinInGet(SYS_CLEAR_PORT, SYS_CLEAR_PIN) == 0)
    {
        LOG(LOG_LEVEL_WARNING, "Clearing state on startup");

        const OSResult status = obc->fs.format(&obc->fs, "/");
        if (OS_RESULT_SUCCEEDED(status))
        {
            LOG(LOG_LEVEL_INFO, "Flash formatted");
        }
        else
        {
            LOGF(LOG_LEVEL_ERROR, "Error formatting flash %d", status);
        }
    }
}

static void SetupAntennas(void)
{
    AntennaMiniportInitialize(&Main.antennaMiniport);
    AntennaDriverInitialize(&Main.antennaDriver, &Main.antennaMiniport, Main.I2C.Bus, Main.I2C.Payload);
}

static void ObcInitTask(void* param)
{
    OBC* obc = (OBC*)param;

    if (!FSInit(&obc->fs, &obc->rootDevice, &obc->rootDeviceDriver))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to initialize file system");
    }

    ClearState(obc);

    if (!TimeInitialize(&obc->timeProvider, NULL, NULL, &obc->fs))
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

    LOG(LOG_LEVEL_INFO, "Intialized");
    Main.initialized = true;
    System.SuspendTask(NULL);
}

void ADXRS(void* param)
{
    UNREFERENCED_PARAMETER(param);
    SPIDRV_HandleData_t handleData;
    SPIDRV_Handle_t handle = &handleData;
    SPIDRV_Init_t initData = ADXRS453_SPI;
    SPIDRV_Init(handle, &initData);
    GyroInterface_t interface;
    interface.writeProc = SPISendB;
    interface.readProc = SPISendRecvB;
    ADXRS453_Obj_t gyro;
    gyro.pinLocations = GYRO0;
    gyro.interface = interface;
    ADXRS453_Obj_t gyro1;
    gyro1.pinLocations = GYRO1;
    gyro1.interface = interface;
    ADXRS453_Obj_t gyro2;
    gyro2.pinLocations = GYRO2;
    gyro2.interface = interface;
    ADXRS453_Init(&gyro, handle);
    ADXRS453_Init(&gyro1, handle);
    ADXRS453_Init(&gyro2, handle);

    while (1)
    {
        SPI_TransferReturn_t rate = ADXRS453_GetRate(&gyro, handle);
        SPI_TransferReturn_t temp = ADXRS453_GetTemperature(&gyro, handle);
        LOGF(LOG_LEVEL_INFO,
            "gyro 0 temp: %d ' celcius rate: %d '/sec rotation\n",
            (int)temp.result.sensorResult,
            (int)rate.result.sensorResult);
        rate = ADXRS453_GetRate(&gyro1, handle);
        temp = ADXRS453_GetTemperature(&gyro1, handle);
        LOGF(LOG_LEVEL_INFO,
            "gyro 1 temp: %d ' celcius rate: %d '/sec rotation\n",
            (int)temp.result.sensorResult,
            (int)rate.result.sensorResult);
        rate = ADXRS453_GetRate(&gyro2, handle);
        temp = ADXRS453_GetTemperature(&gyro2, handle);
        LOGF(LOG_LEVEL_INFO,
            "gyro 2 temp: %d ' celcius rate: %d '/sec rotation\n",
            (int)temp.result.sensorResult,
            (int)rate.result.sensorResult);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void SetupHardware(void)
{
    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_HFCLKLE);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_HFCLKLE);
}

I2CResult I2CErrorHandler(I2CBus* bus, I2CResult result, I2CAddress address, void* context)
{
    UNREFERENCED_PARAMETER(bus);
    UNREFERENCED_PARAMETER(address);

    PowerControl* power = (PowerControl*)context;

    if (result == I2CResultClockLatched)
    {
        LOG(LOG_LEVEL_FATAL, "SCL latched. Triggering power cycle");
        power->TriggerSystemPowerCycle(power);
        return result;
    }

    return result;
}

void SetupI2C(void)
{
    I2CSetupInterface(
        &Main.I2CBuses[0].Bus, I2C0, I2C0_BUS_LOCATION, I2C0_BUS_PORT, I2C0_BUS_SDA_PIN, I2C0_BUS_SCL_PIN, cmuClock_I2C0, I2C0_IRQn);
    I2CSetupInterface(
        &Main.I2CBuses[1].Bus, I2C1, I2C1_BUS_LOCATION, I2C1_BUS_PORT, I2C1_BUS_SDA_PIN, I2C1_BUS_SCL_PIN, cmuClock_I2C1, I2C1_IRQn);

    I2CSetUpErrorHandlingBus(&Main.I2CBuses[0].ErrorHandling, (I2CBus*)&Main.I2CBuses[0].Bus, I2CErrorHandler, &Main.PowerControlInterface);
    I2CSetUpErrorHandlingBus(&Main.I2CBuses[1].ErrorHandling, (I2CBus*)&Main.I2CBuses[1].Bus, I2CErrorHandler, &Main.PowerControlInterface);

    I2CSetUpFallbackBus(&Main.I2CFallback, &Main.I2C);
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

    OSSetup();

    DMADRV_Init();

    LeuartLineIOInit(&Main.IO);

    InitializeTerminal();

    SetupI2C();

    EpsInit((I2CBus*)&Main.I2CFallback);

    EPSPowerControlInitialize(&Main.PowerControlInterface);

    Main.Initialize();

    SwoPutsOnChannel(0, "Hello I'm PW-SAT2 OBC\n");

    SetupAntennas();

    InitializeMission(&Mission, &Main);

    GPIO_PinModeSet(LED_PORT, LED0, gpioModePushPull, 0);
    GPIO_PinModeSet(LED_PORT, LED1, gpioModePushPullDrive, 1);
    GPIO_DriveModeSet(LED_PORT, gpioDriveModeLowest);

    GPIO_PinOutSet(LED_PORT, LED0);
    GPIO_PinOutSet(LED_PORT, LED1);

    System.CreateTask(BlinkLed0, "Blink0", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
    // System.CreateTask(ADXRS, "ADXRS", 512, NULL, tskIDLE_PRIORITY + 2, NULL);
    System.CreateTask(ObcInitTask, "Init", 512, &Main, tskIDLE_PRIORITY + 15, &Main.initTask);
    System.CreateTask(SmartWaitTask, "SmartWait", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
    System.RunScheduler();

    GPIO_PinOutToggle(LED_PORT, LED0);

    return 0;
}
