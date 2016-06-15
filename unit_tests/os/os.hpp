#ifndef OS_INTERFACE_HPP
#define OS_INTERFACE_HPP

#pragma once

#include "base/os.h"

struct IOS
{
    virtual OSResult CreateTask(OSTaskProcedure entryPoint,
        const char* taskName,
        uint16_t stackSize,
        void* taskParameter,
        uint32_t priority,
        OSTaskHandle* taskHandle) = 0;

    virtual void SuspendTask(OSTaskHandle task) = 0;

    virtual void ResumeTask(OSTaskHandle task) = 0;

    virtual void RunScheduller() = 0;

    virtual void Sleep(const OSTaskTimeSpan time) = 0;
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
