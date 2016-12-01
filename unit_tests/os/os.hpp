#ifndef OS_INTERFACE_HPP
#define OS_INTERFACE_HPP

#include <cstdint>

#pragma once

#include "base/os.h"

struct IOS
{
    virtual OSResult CreateTask(OSTaskProcedure entryPoint,
        const char* taskName,
        std::uint16_t stackSize,
        void* taskParameter,
        TaskPriority priority,
        OSTaskHandle* taskHandle) = 0;

    virtual void SuspendTask(OSTaskHandle task) = 0;

    virtual void ResumeTask(OSTaskHandle task) = 0;

    virtual void RunScheduller() = 0;

    virtual void Sleep(const OSTaskTimeSpan time) = 0;

    virtual OSSemaphoreHandle CreateBinarySemaphore() = 0;

    virtual OSResult GiveSemaphore(const OSSemaphoreHandle semaphore) = 0;

    virtual OSResult TakeSemaphore(const OSSemaphoreHandle semaphore, const OSTaskTimeSpan timeout) = 0;

    virtual OSEventGroupHandle CreateEventGroup() = 0;

    virtual OSEventBits EventGroupSetBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange) = 0;

    virtual OSEventBits EventGroupClearBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange) = 0;

    virtual OSEventBits EventGroupWaitForBits(
        OSEventGroupHandle eventGroup, const OSEventBits bitsToWaitFor, bool waitAll, bool autoReset, const OSTaskTimeSpan timeout) = 0;

    virtual void* Alloc(std::size_t size) = 0;
    virtual void Free(void* ptr) = 0;
    virtual OSQueueHandle CreateQueue(std::size_t maxElementCount, std::size_t elementSize) = 0;
    virtual bool QueueReceive(OSQueueHandle queue, void* element, OSTaskTimeSpan timeout) = 0;
    virtual bool QueueReceiveFromISR(OSQueueHandle queue, void* element) = 0;
    virtual bool QueueSend(OSQueueHandle queue, const void* element, OSTaskTimeSpan timeout) = 0;
    virtual bool QueueSendISR(OSQueueHandle queue, const void* element) = 0;
    virtual void QueueOverwrite(OSQueueHandle queue, const void* element) = 0;

    virtual OSPulseHandle CreatePulseAll() = 0;

    virtual OSResult PulseWait(OSPulseHandle handle, OSTaskTimeSpan timeout) = 0;

    virtual void PulseSet(OSPulseHandle handle) = 0;

    virtual void EndSwitchingISR() = 0;

    virtual std::uint32_t GetTickCount() = 0;
};

class OSReset
{
  public:
    OSReset();

    OSReset(OSReset&& arg) noexcept;

    ~OSReset();

    OSReset& operator=(OSReset&& arg) noexcept;

  private:
    bool released;
};

OSReset InstallProxy(IOS* target);

#endif
