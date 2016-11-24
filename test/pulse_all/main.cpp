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
#include "io_map.h"
#include "leuart/leuart.h"
#include "logger/logger.h"
#include "swo/swo.h"
#include "system.h"
#include "terminal/terminal.h"

const int __attribute__((used)) uxTopUsedPriority = configMAX_PRIORITIES;

OSEventGroupHandle GlobalEventGroup;
LineIO StdIO;
Terminal TerminalObject(StdIO);

void SetGroup(uint16_t argc, char* argv[])
{
    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    System::EventGroupSetBits(GlobalEventGroup, 0x80);
}

static const TerminalCommandDescription commands[] = {
    {"SetGroup", SetGroup},
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

static void Observer(void* param)
{
    OSEventGroupHandle groupHandle = *((OSEventGroupHandle*)param);

    for (;;)
    {
        OSEventBits bits = System::EventGroupWaitForBits(groupHandle, 0x80, true, true, MAX_DELAY);
        LOGF(LOG_LEVEL_INFO, "Received notification with following bits: %d", bits);
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

int main(void)
{
    CHIP_Init();

    CMU_ClockEnable(cmuClock_GPIO, true);
    CMU_ClockEnable(cmuClock_DMA, true);

    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFRCO);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFRCO);

    SwoEnable();

    LogInit(LOG_LEVEL_DEBUG);
    InitSwoEndpoint();

    DMADRV_Init();

    LeuartLineIOInit(&StdIO);

    GlobalEventGroup = System::CreateEventGroup();

    for (int i = 0; i < 5; ++i)
    {
        System::CreateTask(Observer, "Observer", 512, &GlobalEventGroup, TaskPriority::P1, NULL);
    }

    InitializeTerminal();

    SwoPutsOnChannel(0, "Ready\n");

    System::RunScheduler();

    return 0;
}
