#ifndef OS_MOCK_HPP
#define OS_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "os/os.hpp"

struct OSMock : IOS
{
    MOCK_METHOD6(CreateTask,
        OSResult(OSTaskProcedure entryPoint,
            const char* taskName,
            uint16_t stackSize,
            void* taskParameter,
            TaskPriority priority,
            OSTaskHandle* taskHandle));

    MOCK_METHOD1(SuspendTask, void(OSTaskHandle task));

    MOCK_METHOD1(ResumeTask, void(OSTaskHandle task));

    MOCK_METHOD0(RunScheduller, void());

    MOCK_METHOD1(Sleep, void(const OSTaskTimeSpan time));

    MOCK_METHOD0(CreateBinarySemaphore, OSSemaphoreHandle());

    MOCK_METHOD2(TakeSemaphore, OSResult(const OSSemaphoreHandle semaphore, const OSTaskTimeSpan time));

    MOCK_METHOD1(GiveSemaphore, OSResult(const OSSemaphoreHandle semaphore));

    MOCK_METHOD0(CreateEventGroup, OSEventGroupHandle());

    MOCK_METHOD2(EventGroupSetBits, OSEventBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange));

    MOCK_METHOD2(EventGroupClearBits, OSEventBits(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange));

    MOCK_METHOD5(EventGroupWaitForBits,
        OSEventBits(
            OSEventGroupHandle eventGroup, const OSEventBits bitsToWaitFor, bool waitAll, bool autoReset, const OSTaskTimeSpan timeout));

    MOCK_METHOD1(Alloc, void*(std::size_t size));
    MOCK_METHOD1(Free, void(void* ptr));
    MOCK_METHOD2(CreateQueue, OSQueueHandle(std::size_t maxElementCount, std::size_t elementSize));
    MOCK_METHOD3(QueueReceive, bool(OSQueueHandle queue, void* element, OSTaskTimeSpan timeout));
    MOCK_METHOD2(QueueReceiveFromISR, bool(OSQueueHandle queue, void* element));
    MOCK_METHOD3(QueueSend, bool(OSQueueHandle queue, const void* element, OSTaskTimeSpan timeout));
    MOCK_METHOD2(QueueSendISR, bool(OSQueueHandle queue, const void* element));
    MOCK_METHOD2(QueueOverwrite, void(OSQueueHandle queue, const void* element));

    MOCK_METHOD0(CreatePulseAll, OSPulseHandle());

    MOCK_METHOD2(PulseWait, OSResult(OSPulseHandle handle, OSTaskTimeSpan timeout));

    MOCK_METHOD1(PulseSet, void(OSPulseHandle handle));

    MOCK_METHOD0(EndSwitchingISR, void());

    MOCK_METHOD0(GetTickCount, std::uint32_t());
};

#endif
