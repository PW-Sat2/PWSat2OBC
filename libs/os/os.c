#include "base/os.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"

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

static OSSemaphoreHandle CreateBinarySemaphore(void)
{
	return xSemaphoreCreateBinary();
}

static uint8_t TakeSemaphore(OSSemaphoreHandle semaphore, OSTaskTimeSpan timeout)
{
	return xSemaphoreTake(semaphore, pdMS_TO_TICKS(timeout));
}

static uint8_t GiveSemaphore(OSSemaphoreHandle semaphore)
{
	return xSemaphoreGive(semaphore);
}

static OSEventGroupHandle CreateEventGroup(void)
{
	return xEventGroupCreate();
}

static OSEventBits EventGroupSetBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange)
{
	return xEventGroupSetBits(eventGroup, bitsToChange);
}

static OSEventBits EventGroupClearBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange)
{
	return xEventGroupClearBits(eventGroup, bitsToChange);
}

static OSEventBits EventGroupWaitForBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToWaitFor, const OSTaskTimeSpan timeout)
{
	return xEventGroupWaitBits(eventGroup, bitsToWaitFor, 0, pdFALSE, timeout);
}

OS System;

OSResult OSSetup(void)
{
    System.CreateTask = TaskCreate;
    System.RunScheduler = RunScheduller;
    System.SleepTask = TaskSleep;
    System.SuspendTask = TaskSuspend;
    System.ResumeTask = TaskResume;
    System.CreateBinarySemaphore = CreateBinarySemaphore;
    System.TakeSemaphore = TakeSemaphore;
    System.GiveSemaphore = GiveSemaphore;
    System.CreateEventGroup = CreateEventGroup;
    System.EventGroupSetBits = EventGroupSetBits;
    System.EventGroupClearBits = EventGroupClearBits;
    System.EventGroupWaitForBits = EventGroupWaitForBits;

    return OSResultSuccess;
}
