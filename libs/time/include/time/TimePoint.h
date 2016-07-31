#ifndef SRC_TIME_POINT_H
#define SRC_TIME_POINT_H

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "system.h"

EXTERNC_BEGIN

typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint16_t day;
    uint16_t milisecond;
} TimePoint;

typedef uint64_t TimeSpan;

typedef int64_t TimeShift;

TimePoint TimePointBuild(uint16_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t milisecond);

TimePoint TimePointFromTimeSpan(TimeSpan span);

TimeSpan TimePointToTimeSpan(TimePoint point);

TimePoint TimePointNormalize(TimePoint point);

bool TimePointEqual(TimePoint left, TimePoint right);

bool TimePointLessThan(TimePoint left, TimePoint right);

static inline bool TimePointNotEqual(TimePoint left, TimePoint right)
{
    return !TimePointEqual(left, right);
}

EXTERNC_END

#endif
