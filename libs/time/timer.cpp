#include <array>

#include "logger/logger.h"
#include "timer.h"

using std::chrono::milliseconds;
using namespace std::chrono_literals;
using namespace services::time;

/**
 * @addtogroup time
 * @{
 */

/**
 * @brief Time period between the subsequent mission time notifications.
 */
static constexpr std::chrono::milliseconds NotificationPeriod(5s);

TimeProvider::TimeProvider()
    : timerLock(nullptr),                 //
      notificationLock(nullptr),          //
      currentTime(0ms),                   //
      notificationTime(0ms),              //
      OnTimePassed(nullptr),              //
      TimePassedCallbackContext(nullptr), //
      TickNotification(nullptr)           //
{
}

bool TimeProvider::Initialize(std::chrono::milliseconds startTime, //
    TimePassedCallbackType timePassedCallback,                     //
    void* timePassedCallbackContext                                //
    )
{
    OnTimePassed = timePassedCallback;
    TimePassedCallbackContext = timePassedCallbackContext;

    currentTime = startTime;
    timerLock = System::CreateBinarySemaphore(TIMER_LOCK_ID);
    notificationLock = System::CreateBinarySemaphore(NOTIFICATION_LOCK_ID);

    if (timerLock != NULL)
    {
        System::GiveSemaphore(timerLock);
    }

    if (notificationLock != NULL)
    {
        System::GiveSemaphore(notificationLock);
    }

    TickNotification = System::CreatePulseAll();

    const bool result = timerLock != NULL //
        && notificationLock != NULL       //
        && TickNotification != NULL;
    return result;
}

void TimeProvider::AdvanceTime(milliseconds delta)
{
    struct TimerState state;

    {
        Lock lock(this->timerLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
            return;
        }

        currentTime = currentTime + delta;
        notificationTime = notificationTime + delta;
        state = BuildTimerState();
    }

    ProcessChange(state);
}

void TimeProvider::Invoke(std::chrono::milliseconds interval)
{
    AdvanceTime(interval);
}

bool TimeProvider::SetCurrentTime(TimePoint pointInTime)
{
    return SetCurrentTime(TimePointToTimeSpan(pointInTime));
}

bool TimeProvider::SetCurrentTime(std::chrono::milliseconds duration)
{
    struct TimerState state;

    {
        Lock lock(this->timerLock, InfiniteTimeout);
        if (!lock())
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
            return false;
        }

        currentTime = duration;
        notificationTime = NotificationPeriod + 1ms;
        state = BuildTimerState();
    }

    ProcessChange(state);
    return true;
}

Option<milliseconds> TimeProvider::GetCurrentTime()
{
    if (this->timerLock == nullptr)
    {
        return None<milliseconds>();
    }

    Lock lock(this->timerLock, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return None<milliseconds>();
    }

    auto copy = currentTime;
    return Some(copy);
}

Option<TimePoint> TimeProvider::GetCurrentMissionTime()
{
    const Option<milliseconds> result = GetCurrentTime();
    if (result.HasValue)
    {
        return Some(TimePointFromDuration(result.Value));
    }

    return None<TimePoint>();
}

void TimeProvider::ProcessChange(TimerState state)
{
    Lock lock(this->notificationLock, InfiniteTimeout);
    if (!lock())
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire notification lock.");
        return;
    }

    SendTimeNotification(state);
}

TimerState TimeProvider::BuildTimerState()
{
    struct TimerState result;
    result.time = currentTime;
    result.sendNotification = NotificationPeriod < notificationTime;
    if (result.sendNotification)
    {
        notificationTime = 0ms;
    }

    return result;
}

void TimeProvider::SendTimeNotification(TimerState state)
{
    if (state.sendNotification && OnTimePassed != NULL)
    {
        OnTimePassed(TimePassedCallbackContext);
    }

    if (state.sendNotification)
    {
        System::PulseSet(TickNotification);
    }
}

bool TimeProvider::LongDelayUntil(TimePoint time)
{
    do
    {
        Option<TimePoint> missionTime = GetCurrentMissionTime();

        if (!missionTime.HasValue)
        {
            return false;
        }

        if (TimePointLessThan(time, missionTime.Value))
        {
            return true;
        }

        if (TimePointEqual(time, missionTime.Value))
        {
            return true;
        }

        if (OS_RESULT_FAILED(System::PulseWait(TickNotification, InfiniteTimeout)))
        {
            return false;
        }
    } while (true);

    return false;
}

bool TimeProvider::LongDelay(milliseconds delay)
{
    Option<TimePoint> missionTime = GetCurrentMissionTime();

    if (!missionTime.HasValue)
    {
        return false;
    }

    TimePoint time = TimePointFromDuration(TimePointToTimeSpan(missionTime.Value) + delay);

    return LongDelayUntil(time);
}

/** @} */
