#ifndef SRC_TIMER_H
#define SRC_TIMER_H

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "TimePoint.h"
#include "base/os.h"
#include "fs/fs.h"
#include "system.h"

EXTERNC_BEGIN

struct TimeProvider;

typedef void (*TimeTickCallbackType)(struct TimeProvider* provider, TimeSpan delta);

typedef void (*TimePassedCallbackType)(void* context, TimePoint currentTime);

struct TimeSnapshot
{
    TimeSpan CurrentTime;
};

struct TimeProvider
{
    TimePassedCallbackType OnTimePassed;

    void* TimePassedCallbackContext;

    TimeSpan CurrentTime;

    TimeSpan NotificationTime;

    TimeSpan PersistanceTime;

    OSSemaphoreHandle timerLock;

    OSSemaphoreHandle notificationLock;

    FileSystem* FileSystemObject;
};

bool TimeInitialize(
    struct TimeProvider* provider, TimePassedCallbackType timePassedCallback, void* timePassedCallbackContext, FileSystem* fileSystem);

TimeSpan TimeGetCurrentTime(struct TimeProvider* timeProvider);

TimePoint TimeGetCurrentMissionTime(struct TimeProvider* timeProvider);

void TimeAdvanceTime(struct TimeProvider* timeProvider, TimeSpan delta);

bool TimeSetCurrentTime(struct TimeProvider* timeProvider, TimePoint pointInTime);

TimeTickCallbackType TimeGetTickProcedure(void);

struct TimeSnapshot GetCurrentPersistentTime(FileSystem* fileSystem);

static inline bool TimeSnapshotEqual(struct TimeSnapshot left, struct TimeSnapshot right)
{
    return left.CurrentTime == right.CurrentTime;
}

static inline bool TimeSnapshotLessThan(struct TimeSnapshot left, struct TimeSnapshot right)
{
    return left.CurrentTime < right.CurrentTime;
}

EXTERNC_END

#endif
