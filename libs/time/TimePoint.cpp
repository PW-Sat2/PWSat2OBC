#include "TimePoint.h"

TimeSpan TimeSpanFromMilliseconds(uint64_t milliseconds)
{
    return std::chrono::duration_cast<TimeSpan>(std::chrono::milliseconds(milliseconds));
}

TimeSpan TimeSpanFromSeconds(uint32_t seconds)
{
    return std::chrono::duration_cast<TimeSpan>(std::chrono::seconds(seconds));
}

TimeSpan TimeSpanFromMinutes(uint32_t minutes)
{
    return std::chrono::duration_cast<TimeSpan>(std::chrono::minutes(minutes));
}

TimeSpan TimeSpanFromHours(uint32_t hours)
{
    return std::chrono::duration_cast<TimeSpan>(std::chrono::hours(hours));
}

TimeSpan TimeSpanFromDays(uint32_t days)
{
    return std::chrono::duration_cast<TimeSpan>(std::chrono::hours(days * 24));
}

uint32_t TimeSpanToSeconds(TimeSpan span)
{
    return std::chrono::duration_cast<std::chrono::seconds>(span).count();
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

TimePoint TimePointFromTimeSpan(const TimeSpan timeSpan)
{
    TimePoint point = {};
    uint64_t span = std::chrono::duration_cast<std::chrono::milliseconds>(timeSpan).count();
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
    uint64_t result = point.day;
    result *= 24;
    result += point.hour;
    result *= 60;
    result += point.minute;
    result *= 60;
    result += point.second;
    result *= 1000;
    result += point.milisecond;
    return TimeSpanFromMilliseconds(result);
}
