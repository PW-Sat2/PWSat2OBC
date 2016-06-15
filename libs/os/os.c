#include "base/os.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

static OSResult TaskCreate(OSTaskProcedure entryPoint,
    const char* taskName,
    uint16_t stackSize,
    void* taskParameter,
    uint32_t priority,
    OSTaskHandle* taskHandle)
{
    const BaseType_t result = xTaskCreate(entryPoint, taskName, stackSize, taskParameter, priority, taskHandle);
    return result == pdPASS ? OSResultSuccess : OSResultOutOfResources;
}

static void RunScheduller(void)
{
    vTaskStartScheduler();
}

static void TaskSleep(const OSTaskTimeSpan time)
{
    vTaskDelay(pdMS_TO_TICKS(time));
}

static void TaskSuspend(OSTaskHandle task)
{
    vTaskSuspend(task);
}

static void TaskResume(OSTaskHandle task)
{
    vTaskResume(task);
}

OS System;

OSResult OSSetup(void)
{
    System.CreateTask = TaskCreate;
    System.RunScheduler = RunScheduller;
    System.SleepTask = TaskSleep;
    System.SuspendTask = TaskSuspend;
    System.ResumeTask = TaskResume;
    return OSResultSuccess;
}
