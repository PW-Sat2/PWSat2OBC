#ifndef SRC_TIMER_H
#define SRC_TIMER_H

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "TimePoint.h"
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
};

bool TimeInitialize(struct TimeProvider* provider, TimePassedCallbackType timePassedCallback, void* timePassedCallbackContext);

static TimeSpan TimeGetCurrentTime(struct TimeProvider* timeProvider);

static TimePoint TimeGetCurrentMissionTime(struct TimeProvider* timeProvider);

void TimeAdvanceTime(struct TimeProvider* timeProvider, TimeSpan delta);

void TimeSetCurrentTime(struct TimeProvider* timeProvider, TimePoint pointInTime);

TimeTickCallbackType TimeGetTickProcedure(void);

struct TimeSnapshot GetCurrentPersistentTime(void);

static inline TimeSpan TimeGetCurrentTime(struct TimeProvider* timeProvider)
{
    return timeProvider->CurrentTime;
}

static inline TimePoint TimeGetCurrentMissionTime(struct TimeProvider* timeProvider)
{
    return TimePointFromTimeSpan(timeProvider->CurrentTime);
}

EXTERNC_END

#endif
