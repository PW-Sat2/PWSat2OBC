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
                     uint32_t priority,
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
        OSEventBits(OSEventGroupHandle eventGroup,
                     const OSEventBits bitsToWaitFor,
                     bool waitAll,
                     bool autoReset,
                     const OSTaskTimeSpan timeout));
};

#endif
