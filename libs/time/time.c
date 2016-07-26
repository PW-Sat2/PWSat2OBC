#include "time.h"

static TimeSpan GetCurrentPersistentTime(void);

static void TimeTickProcedure(struct TimeProvider* provider, TimeSpan delta);

bool TimeInitialize(struct TimeProvider* provider, OnTimePassedCallbackType timePassedCallback, void* timePassedCallbackContext)
{
    provider->currentTime = GetCurrentPersistentTime();
    provider->OnTimePassed = timePassedCallback;
    provider->TimePassedCallbackContext = timePassedCallbackContext;
    return true;
}

OnTimeTickCallbackType TimeGetTickProcedure(void)
{
    return TimeTickProcedure;
}

int TimePointCompare(TimePoint left, TimePoint right);

bool TimeAdvanceTime(struct TimeProvider* timeProvider, TimeShift delta)
{
    if (delta < 0)
    {
        return false;
    }
    else
    {
        timeProvider->currentTime += delta;
        timeProvider->OnTimePassed(timeProvider->TimePassedCallbackContext, TimePointFromTimeSpan(timeProvider->currentTime));
        return true;
    }
}

void TimeSetCurrentTime(struct TimeProvider* timeProvider, struct TimePoint pointInTime)
{
    timeProvider->currentTime = pointInTime;
    timeProvider->OnTimePassed(timeProvider->TimePassedCallbackContext, pointInTime);
}

struct TimePoint TimePointFromTimeSpan(TimeSpan span)
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

void TimeTickProcedure(struct TimeProvider* provider, TimeSpan delta)
{
    provider->currentTime += delta;
    provider->OnTimePassed(provider->TimePassedCallbackContext, TimePointFromTimeSpan(provider->currentTime));
}

TimeSpan GetCurrentPersistentTime()
{
    return 0;
}
