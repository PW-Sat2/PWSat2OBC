#ifndef LIBS_BASE_OS_H
#define LIBS_BASE_OS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define MAX_DELAY 0xffffffffUL

typedef enum {
    OSResultSuccess = 0,
    OSResultOutOfResources = 1,
    OSResultTimeout = 2,
} OSResult;

typedef uint16_t OSTaskTimeSpan;

typedef void (*OSTaskProcedure)(void* param);

typedef void* OSTaskHandle;
typedef void* OSSemaphoreHandle;

typedef OSResult (*OSTaskCreateProc)(OSTaskProcedure entryPoint,
    const char* taskName,
    uint16_t stackSize,
    void* taskParameter,
    uint32_t priority,
    OSTaskHandle* taskHandle);

typedef void (*OSTaskProcedure)(OSTaskHandle task);

typedef void (*OSGenericProc)(void);

typedef void (*OSTaskSleepProc)(const OSTaskTimeSpan time);

typedef OSSemaphoreHandle (*OSCreateSemaphore)(void);

typedef uint8_t (*OSTakeSemaphore)(OSSemaphoreHandle semaphore, OSTaskTimeSpan timeout);
typedef uint8_t (*OSGiveSemaphore)(OSSemaphoreHandle semaphore);

typedef struct
{
    OSTaskCreateProc CreateTask;
    OSTaskSleepProc SleepTask;
    OSTaskProcedure SuspendTask;
    OSTaskProcedure ResumeTask;
    OSGenericProc RunScheduler;
    OSCreateSemaphore CreateBinarySemaphore;
    OSTakeSemaphore TakeSemaphore;
    OSGiveSemaphore GiveSemaphore;

} OS;

OSResult OSSetup(void);

extern OS System;

#ifdef __cplusplus
}
#endif

#endif
