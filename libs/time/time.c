#include "TimePoint.h"

TimePoint TimePointBuild(uint16_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond)
{
    TimePoint point;
    point.milisecond = millisecond;
    point.second = second;
    point.minute = minute;
    point.hour = hour;
    point.day = day;
    return point;
}

TimePoint TimePointNormalize(TimePoint point)
{
    return TimePointFromTimeSpan(TimePointToTimeSpan(point));
}

TimePoint TimePointFromTimeSpan(TimeSpan span)
{
    TimePoint point = {0};
    point.milisecond = span % 1000;
    span /= 1000;
    point.second = span % 60;
    span /= 60;
    point.minute = span % 60;
    span /= 60;
    point.hour = span % 24;
    point.day = span / 24;
    return point;
}

TimeSpan TimePointToTimeSpan(TimePoint point)
{
    TimeSpan result = point.day;
    result *= 24;
    result += point.hour;
    result *= 60;
    result += point.minute;
    result *= 60;
    result += point.second;
    result *= 1000;
    result += point.milisecond;
    return result;
}
