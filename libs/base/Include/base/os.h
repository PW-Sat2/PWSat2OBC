#ifndef LIBS_BASE_OS_H
#define LIBS_BASE_OS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum {
    OSResultSuccess = 0,
    OSResultOutOfResources = 1,
    OSResultTimeout = 2,
} OSResult;

typedef uint16_t OSTaskTimeSpan;

typedef void (*OSTaskProcedure)(void* param);

typedef void* OSTaskHandle;

typedef OSResult (*OSTaskCreateProc)(OSTaskProcedure entryPoint,
    const char* taskName,
    uint16_t stackSize,
    void* taskParameter,
    uint32_t priority,
    OSTaskHandle* taskHandle);

typedef void (*OSTaskProcedure)(OSTaskHandle task);

typedef void (*OSGenericProc)(void);

typedef void (*OSTaskSleepProc)(const OSTaskTimeSpan time);

typedef struct
{
    OSTaskCreateProc CreateTask;
    OSTaskSleepProc SleepTask;
    OSTaskProcedure SuspendTask;
    OSTaskProcedure ResumeTask;
    OSGenericProc RunScheduler;
} OS;

OSResult OSSetup(void);

extern OS System;

#ifdef __cplusplus
}
#endif

#endif
