#ifndef SRC_TIME_H
#define SRC_TIME_H

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "system.h"

EXTERNC_BEGIN

typedef struct
{
    uint16_t milisecond;
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
} TimePoint;

typedef uint32_t TimeSpan;

typedef int32_t TimeShift;

typedef void (*OnTimeTickCallbackType)(struct TimeProvider* provider, TimeSpan delta);

typedef void (*OnTimePassedCallbackType)(void* context, TimePoint currentTime);

struct TimeProvider
{
    OnTimePassedCallbackType OnTimePassed;

    void* TimePassedCallbackContext;

    TimeSpan currentTime;
};

bool TimeInitialize(struct TimeProvider* provider, OnTimePassedCallbackType timePassedCallback, void* timePassedCallbackContext);

TimeSpan TimeGetCurrentTime(struct TimeProvider* timeProvider);

struct TimePoint TimeGetCurrentMissionTime(struct TimeProvider* timeProvider);

bool TimeAdvanceTime(struct TimeProvider* timeProvider, TimeShift delta);

void TimeSetCurrentTime(struct TimeProvider* timeProvider, struct TimePoint pointInTime);

OnTimeTickCallbackType TimeGetTickProcedure(void);

struct TimePoint TimePointFromTimeSpan(TimeSpan span);

TimeSpan TimePointToTimeSpan(struct TimePoint point);

static inline TimeSpan TimeGetCurrentTime(struct TimeProvider* timeProvider)
{
    return timeProvider->currentTime;
}

static inline struct TimePoint TimeGetCurrentMissionTime(struct TimeProvider* timeProvider)
{
    return TimePointFromTimeSpan(timeProvider->currentTime);
}

EXTERNC_END

#endif
