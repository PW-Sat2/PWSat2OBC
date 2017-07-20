#include <stdio.h>
#include <string.h>
#include <em_chip.h>
#include <em_cmu.h>
#include <em_emu.h>
#include <em_usart.h>

#include <FreeRTOS.h>
#include <FreeRTOSConfig.h>
#include <task.h>

#include <array>
#include "SwoEndpoint/SwoEndpoint.h"
#include "base/os.h"
#include "dmadrv.h"
#include "logger/logger.h"
#include "mcu/io_map.h"
#include "n25q/n25q.h"
#include "payload/io_map.h"
#include "spi/efm.h"
#include "swo/swo.h"
#include "system.h"

using namespace std::chrono_literals;

using namespace devices::n25q;

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

static drivers::spi::EFMSPIInterface SPI;
static drivers::gpio::OutputPin<io_map::SlaveSelectFlash1> SlaveSelect;
static drivers::spi::EFMSPISlaveInterface SlaveSPI(SPI, SlaveSelect);

extern "C" void vApplicationIdleHook(void)
{
    //    EMU_EnterEM1();
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

static void Ping(void* v)
{
    UNREFERENCED_PARAMETER(v);

    while (1)
    {
        GPIO_PinOutToggle(io_map::BootIndicator::Port, io_map::BootIndicator::PinNumber);

        System::SleepTask(1s);
    }
}

struct BootErrorCounter : public error_counter::IErrorCounting
{
    virtual error_counter::CounterValue Current(error_counter::Device /* device*/) const override
    {
        return 0;
    }

    virtual void Failure(error_counter::Device /*device*/) override
    {
    }

    virtual void Success(error_counter::Device /*device*/) override
    {
    }
};

extern void Hang();

static void Test(void*)
{
    SPI.Initialize();
    SlaveSelect.Initialize();

    BootErrorCounter ec;
    N25QDriver flash(ec, 1, SlaveSPI);

    auto r = flash.Reset();
    LOGF(LOG_LEVEL_INFO, "Reset: %d", num(r));

    while (true)
    {
        auto wait = flash.BeginEraseSector(0x0);
        r = wait.Wait();
        LOGF(LOG_LEVEL_INFO, "Erase result %d", num(r));

        std::array<std::uint8_t, 256> writeBuffer;
        std::array<std::uint8_t, 256> readBuffer;

        for (int i = 0; i < 256; i++)
        {
            writeBuffer[i] = i;
        }

        readBuffer.fill(0);

        wait = flash.BeginWritePage(0x0, 0x0, writeBuffer);
        r = wait.Wait();
        LOGF(LOG_LEVEL_INFO, "Write result %d", num(r));

        flash.ReadMemory(0x0, readBuffer);

        if (memcmp(writeBuffer.data(), readBuffer.data(), readBuffer.size()) == 0)
        {
            LOG(LOG_LEVEL_INFO, "CMP OK");
        }
        else
        {
            LOG(LOG_LEVEL_INFO, "CMP NOT OK");
        }
    }

    while (true)
    {
        auto id = flash.ReadId();

        if (!id.IsValid())
        {
            LOGF(LOG_LEVEL_INFO, "INVALID ID: %X %X %X", id.Manufacturer, id.MemoryType, id.MemoryCapacity);
        }
    }

    System::SuspendTask(nullptr);
}

extern "C" void __libc_init_array(void);

int main(void)
{
    __libc_init_array();

    CHIP_Init();

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

    SwoPutsOnChannel(0, "Flash test Ready\n");

    DMADRV_Init();

    GPIO_PinModeSet(io_map::BootIndicator::Port, io_map::BootIndicator::PinNumber, gpioModePushPull, 0);

    GPIO_PinOutSet(io_map::BootIndicator::Port, io_map::BootIndicator::PinNumber);

    System::CreateTask(Test, "Test", 10_KB, NULL, TaskPriority::P1, NULL);
    System::CreateTask(Ping, "Ping", 1_KB, NULL, TaskPriority::P1, NULL);

    System::RunScheduler();

    return 0;
}
