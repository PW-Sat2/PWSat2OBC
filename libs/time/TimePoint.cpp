#include "TimePoint.h"

using std::chrono::milliseconds;
using std::chrono::duration_cast;

milliseconds TimeSpanFromDays(uint32_t days)
{
    return duration_cast<milliseconds>(std::chrono::hours(days * 24));
}

TimePoint TimePointBuild(uint16_t day, uint8_t hour, uint8_t minute, uint8_t second, uint16_t millisecond)
{
    TimePoint point;
    point.milisecond = millisecond;
    point.second = second;
    point.minute = minute;
    point.hour = hour;
    point.day = day;
    return TimePointNormalize(point);
}

TimePoint TimePointNormalize(TimePoint point)
{
    return TimePointFromTimeSpan(TimePointToTimeSpan(point));
}

TimePoint TimePointFromTimeSpan(const milliseconds timeSpan)
{
    TimePoint point = {};
    uint64_t span = timeSpan.count();
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

milliseconds TimePointToTimeSpan(TimePoint point)
{
    uint64_t result = point.day;
    result *= 24;
    result += point.hour;
    result *= 60;
    result += point.minute;
    result *= 60;
    result += point.second;
    result *= 1000;
    result += point.milisecond;
    return milliseconds(result);
}
