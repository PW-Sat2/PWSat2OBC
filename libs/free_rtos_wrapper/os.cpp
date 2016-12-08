#include "base/os.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define PULSE_ALL_BITS 0x80

static_assert(static_cast<uint8_t>(TaskPriority::Idle) == 0, "Idle priority must be 0");
static_assert(static_cast<uint8_t>(TaskPriority::Highest) < configMAX_PRIORITIES, "Priorites up to configMAX_PRIORITIES - 1 are allowed");

static inline TickType_t ConvertTimeToTicks(const OSTaskTimeSpan span)
{
    if (span == MAX_DELAY)
    {
        return portMAX_DELAY;
    }

    const uint64_t time = span;
    return pdMS_TO_TICKS(time);
}

OSResult System::CreateTask(OSTaskProcedure entryPoint, //
    const char* taskName,                               //
    std::uint16_t stackSize,                            //
    void* taskParameter,                                //
    TaskPriority priority,                              //
    OSTaskHandle* taskHandle                            //
    )
{
    const BaseType_t result = xTaskCreate(entryPoint, taskName, stackSize, taskParameter, static_cast<uint8_t>(priority), taskHandle);
    if (result != pdPASS)
    {
        return OSResult::NotEnoughMemory;
    }
    else
    {
        return OSResult::Success;
    }
}

void System::RunScheduler(void)
{
    vTaskStartScheduler();
}

void System::SleepTask(const OSTaskTimeSpan time)
{
    vTaskDelay(ConvertTimeToTicks(time));
}

void System::SuspendTask(OSTaskHandle task)
{
    vTaskSuspend(task);
}

void System::ResumeTask(OSTaskHandle task)
{
    vTaskResume(task);
}

OSSemaphoreHandle System::CreateBinarySemaphore(void)
{
    return xSemaphoreCreateBinary();
}

OSResult System::TakeSemaphore(OSSemaphoreHandle semaphore, OSTaskTimeSpan timeout)
{
    const BaseType_t result = xSemaphoreTake(semaphore, ConvertTimeToTicks(timeout));
    if (result != pdPASS)
    {
        return OSResult::Timeout;
    }
    else
    {
        return OSResult::Success;
    }
}

OSResult System::GiveSemaphore(OSSemaphoreHandle semaphore)
{
    const BaseType_t result = xSemaphoreGive(semaphore);
    if (result != pdPASS)
    {
        return OSResult::InvalidOperation;
    }
    else
    {
        return OSResult::Success;
    }
}

OSEventGroupHandle System::CreateEventGroup(void)
{
    return xEventGroupCreate();
}

OSEventBits System::EventGroupSetBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange)
{
    return xEventGroupSetBits(eventGroup, bitsToChange);
}

OSEventBits System::EventGroupClearBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange)
{
    return xEventGroupClearBits(eventGroup, bitsToChange);
}

OSEventBits System::EventGroupWaitForBits(OSEventGroupHandle eventGroup, //
    const OSEventBits bitsToWaitFor,                                     //
    bool waitAll,                                                        //
    bool autoReset,                                                      //
    const OSTaskTimeSpan timeout                                         //
    )
{
    return xEventGroupWaitBits(eventGroup, //
        bitsToWaitFor,                     //
        autoReset ? bitsToWaitFor : 0,     //
        waitAll ? pdTRUE : pdFALSE,        //
        ConvertTimeToTicks(timeout)        //
        );
}

void* System::Alloc(size_t size)
{
    return pvPortMalloc(size);
}

void System::Free(void* ptr)
{
    UNREFERENCED_PARAMETER(ptr);
    // Free is not suported
}

OSQueueHandle System::CreateQueue(size_t maxElementCount, size_t elementSize)
{
    return xQueueCreate(maxElementCount, elementSize);
}

bool System::QueueReceive(OSQueueHandle queue, void* element, OSTaskTimeSpan timeout)
{
    return xQueueReceive(queue, element, ConvertTimeToTicks(timeout)) == pdTRUE;
}

bool System::QueueReceiveFromISR(OSQueueHandle queue, void* element)
{
    bool result = xQueueReceiveFromISR(queue, element, nullptr) == pdTRUE;

    return result;
}

bool System::QueueSend(OSQueueHandle queue, const void* element, OSTaskTimeSpan timeout)
{
    return xQueueSend(queue, element, ConvertTimeToTicks(timeout)) == pdTRUE;
}

bool System::QueueSendISR(OSQueueHandle queue, const void* element)
{
    bool result = xQueueSendFromISR(queue, element, nullptr) == pdTRUE;

    return result;
}

void System::QueueOverwrite(OSQueueHandle queue, const void* element)
{
    xQueueOverwrite(queue, element);
}

void System::EndSwitchingISR()
{
    portEND_SWITCHING_ISR(nullptr);
}

OSPulseHandle System::CreatePulseAll(void)
{
    return (OSPulseHandle)System::CreateEventGroup();
}

OSResult System::PulseWait(OSPulseHandle handle, OSTaskTimeSpan timeout)
{
    OSEventBits result = System::EventGroupWaitForBits((OSEventGroupHandle)handle, PULSE_ALL_BITS, true, true, timeout);

    if (result == PULSE_ALL_BITS)
    {
        return OSResult::Success;
    }
    else
    {
        return OSResult::Timeout;
    }
}

void System::PulseSet(OSPulseHandle handle)
{
    System::EventGroupSetBits((OSEventGroupHandle)handle, PULSE_ALL_BITS);
}
