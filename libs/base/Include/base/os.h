#ifndef LIBS_BASE_OS_H
#define LIBS_BASE_OS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define MAX_DELAY 0xffffffffUL

typedef enum {
    OSResultSuccess = 0,
    OSResultOutOfResources = 1,
    OSResultTimeout = 2,
    OSResultInvalidOperation = 3,
} OSResult;

typedef uint32_t OSTaskTimeSpan;

typedef void (*OSTaskProcedure)(void* param);

typedef void* OSTaskHandle;
typedef void* OSSemaphoreHandle;
typedef void* OSEventGroupHandle;
typedef uint32_t OSEventBits;

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

typedef OSResult (*OSTakeSemaphore)(OSSemaphoreHandle semaphore, OSTaskTimeSpan timeout);

typedef OSResult (*OSGiveSemaphore)(OSSemaphoreHandle semaphore);

typedef OSEventGroupHandle (*OSCreateEventGroup)(void);

typedef OSEventBits (*OSEventGroupChangeBits)(OSEventGroupHandle eventGroup, const OSEventBits bitsToChange);

typedef OSEventBits (*OSEventGroupWaitForBits)(OSEventGroupHandle eventGroup,
    const OSEventBits bitsToWaitFor,
    bool waitAll,
    bool autoReset,
    const OSTaskTimeSpan timeout);

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
    OSCreateEventGroup CreateEventGroup;
    OSEventGroupChangeBits EventGroupSetBits;
    OSEventGroupChangeBits EventGroupClearBits;
    OSEventGroupWaitForBits EventGroupWaitForBits;
} OS;

OSResult OSSetup(void);

extern OS System;

#ifdef __cplusplus
}
#endif

#endif
