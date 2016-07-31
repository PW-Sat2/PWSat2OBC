#include "timer.h"

static const TimeSpan NotificationPeriod = 10000; // 10s

static const TimeSpan SavePeriod = 15 * 60 * 1000; // 15 min

static void TimeTickProcedure(struct TimeProvider* provider, TimeSpan delta);

static void SendTimeNotification(struct TimeProvider* provider);

static void SaveTime(struct TimeProvider* provider);

bool TimeInitialize(struct TimeProvider* provider, TimePassedCallbackType timePassedCallback, void* timePassedCallbackContext)
{
    const struct TimeSnapshot snapshot = GetCurrentPersistentTime();
    provider->CurrentTime = snapshot.CurrentTime;
    provider->NotificationTime = 0;
    provider->PersistanceTime = 0;
    provider->OnTimePassed = timePassedCallback;
    provider->TimePassedCallbackContext = timePassedCallbackContext;
    return true;
}

TimeTickCallbackType TimeGetTickProcedure(void)
{
    return TimeTickProcedure;
}

void TimeAdvanceTime(struct TimeProvider* timeProvider, TimeSpan delta)
{
    TimeTickProcedure(timeProvider, delta);
}

bool TimePointLessThan(TimePoint left, TimePoint right)
{
    return TimePointToTimeSpan(left) < TimePointToTimeSpan(right);
}

void TimeSetCurrentTime(struct TimeProvider* timeProvider, TimePoint pointInTime)
{
    const TimeSpan span = TimePointToTimeSpan(pointInTime);
    timeProvider->CurrentTime = span;
    timeProvider->NotificationTime = span;
    timeProvider->PersistanceTime = span;
    TimeTickProcedure(timeProvider, 0);
}

void TimeTickProcedure(struct TimeProvider* provider, TimeSpan delta)
{
    provider->CurrentTime += delta;
    provider->NotificationTime += delta;
    provider->PersistanceTime += delta;
    SendTimeNotification(provider);
    SaveTime(provider);
}

struct TimeSnapshot GetCurrentPersistentTime()
{
    struct TimeSnapshot snapshot = {0};
    // TODO
    return snapshot;
}

static void SendTimeNotification(struct TimeProvider* timeProvider)
{
    if (timeProvider->NotificationTime > NotificationPeriod)
    {
        timeProvider->OnTimePassed(timeProvider->TimePassedCallbackContext, TimePointFromTimeSpan(timeProvider->CurrentTime));
        timeProvider->NotificationTime = 0;
    }
}

static void SaveTime(struct TimeProvider* timeProvider)
{
    if (timeProvider->PersistanceTime > SavePeriod)
    {
        // TODO
        timeProvider->PersistanceTime = 0;
    }
}
