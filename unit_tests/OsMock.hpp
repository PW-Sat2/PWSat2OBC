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
};

#endif
