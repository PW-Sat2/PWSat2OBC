#include "base/os.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "event_groups.h"
#include "logger/logger.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

using std::chrono::milliseconds;

#define PULSE_ALL_BITS 0x80

static_assert(static_cast<uint8_t>(TaskPriority::Idle) == 0, "Idle priority must be 0");
static_assert(static_cast<uint8_t>(TaskPriority::Highest) < configMAX_PRIORITIES, "Priorites up to configMAX_PRIORITIES - 1 are allowed");

static inline TickType_t ConvertTimeToTicks(const milliseconds span)
{
    if (span == InfiniteTimeout)
    {
        return portMAX_DELAY;
    }

    return pdMS_TO_TICKS(span.count());
}

OSResult System::CreateTask(OSTaskProcedure entryPoint, //
    const char* taskName,                               //
    std::uint16_t stackSize,                            //
    void* taskParameter,                                //
    TaskPriority priority,                              //
    OSTaskHandle* taskHandle                            //
    )
{
    const auto rtosStackSize = stackSize / sizeof(StackType_t);

    const BaseType_t result = xTaskCreate(entryPoint, taskName, rtosStackSize, taskParameter, static_cast<uint8_t>(priority), taskHandle);
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

void System::SleepTask(const milliseconds time)
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

OSSemaphoreHandle System::CreateBinarySemaphore(uint8_t semaphoreId)
{
    UNREFERENCED_PARAMETER(semaphoreId);
    auto s = xSemaphoreCreateBinary();

    if (s == 0)
    {
        LOG(LOG_LEVEL_FATAL, "Unable to create binary semaphore");
    }

    return s;
}

OSSemaphoreHandle System::CreateBinarySemaphore(OSSemaphoreBuffer& semaphoreBuffer)
{
    return xSemaphoreCreateBinaryStatic(&semaphoreBuffer);
}

OSResult System::TakeSemaphore(OSSemaphoreHandle semaphore, milliseconds timeout)
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

OSResult System::GiveSemaphoreISR(OSSemaphoreHandle semaphore)
{
    const BaseType_t result = xSemaphoreGiveFromISR(semaphore, nullptr);
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

OSEventBits System::EventGroupGetBits(OSEventGroupHandle eventGroup)
{
    return xEventGroupGetBits(eventGroup);
}

OSEventBits System::EventGroupSetBitsISR(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange)
{
    return xEventGroupSetBitsFromISR(eventGroup, bitsToChange, nullptr);
}

OSEventBits System::EventGroupClearBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange)
{
    return xEventGroupClearBits(eventGroup, bitsToChange);
}

OSEventBits System::EventGroupWaitForBits(OSEventGroupHandle eventGroup, //
    const OSEventBits bitsToWaitFor,                                     //
    bool waitAll,                                                        //
    bool autoReset,                                                      //
    const milliseconds timeout                                           //
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

bool System::QueueReceive(OSQueueHandle queue, void* element, milliseconds timeout)
{
    return xQueueReceive(queue, element, ConvertTimeToTicks(timeout)) == pdTRUE;
}

bool System::QueueReceiveFromISR(OSQueueHandle queue, void* element)
{
    bool result = xQueueReceiveFromISR(queue, element, nullptr) == pdTRUE;

    return result;
}

bool System::QueueSend(OSQueueHandle queue, const void* element, milliseconds timeout)
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

OSResult System::PulseWait(OSPulseHandle handle, milliseconds timeout)
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

milliseconds System::GetUptime()
{
    typedef std::ratio<portTICK_PERIOD_MS, 1000> ticksPerMs;
    typedef std::chrono::duration<int64_t, ticksPerMs> ticks;

    return std::chrono::duration_cast<milliseconds>(ticks(xTaskGetTickCount()));
}

void System::Yield()
{
    portYIELD();
}
