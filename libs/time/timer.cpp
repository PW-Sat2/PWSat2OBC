#include "timer.h"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"

using namespace services::time;

/**
 * @addtogroup time
 * @{
 */

/**
 * @brief Time period between the subsequent mission time notifications.
 */
static constexpr TimeSpan NotificationPeriod = {TIMER_NOTIFICATION_PERIOD};

/**
 * @brief Time period between subsequent timer state saves.
 */
static constexpr TimeSpan SavePeriod = {TIMER_SAVE_PERIOD};

bool TimeSnapshot::operator==(const TimeSnapshot& right) const
{
    return TimeSpanEqual(CurrentTime, right.CurrentTime);
}

bool TimeSnapshot::operator!=(const TimeSnapshot& right) const
{
    return !(*this == right);
}

bool TimeSnapshot::operator<(const TimeSnapshot& right) const
{
    return TimeSpanLessThan(CurrentTime, right.CurrentTime);
}

bool TimeSnapshot::operator>(const TimeSnapshot& right) const
{
    return right < *this;
}

bool TimeSnapshot::operator<=(const TimeSnapshot& right) const
{
    return !(*this > right);
}

bool TimeSnapshot::operator>=(const TimeSnapshot& right) const
{
    return !(*this < right);
}

TimeProvider::TimeProvider(FileSystem& fileSystem)
    : timerLock(nullptr),                                                                   //
      notificationLock(nullptr),                                                            //
      OnTimePassed(nullptr),                                                                //
      TimePassedCallbackContext(nullptr), NotificationTime(TimeSpanFromMilliseconds(0ull)), //
      PersistanceTime(TimeSpanFromMilliseconds(0ull)),                                      //
      TickNotification(nullptr),                                                            //
      FileSystemObject(&fileSystem)                                                         //
{
}

bool TimeProvider::Initialize(TimePassedCallbackType timePassedCallback, void* timePassedCallbackContext)
{
    OnTimePassed = timePassedCallback;
    TimePassedCallbackContext = timePassedCallbackContext;

    const struct TimeSnapshot snapshot = CurrentPersistentTime(FileSystemObject);
    CurrentTime = snapshot.CurrentTime;
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

void TimeProvider::AdvanceTime(TimeSpan delta)
{
    if (OS_RESULT_FAILED(System::TakeSemaphore(timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return;
    }

    CurrentTime = TimeSpanAdd(CurrentTime, delta);
    NotificationTime = TimeSpanAdd(NotificationTime, delta);
    PersistanceTime = TimeSpanAdd(PersistanceTime, delta);

    struct TimerState state = BuildTimerState();

    System::GiveSemaphore(timerLock);
    ProcessChange(state);
}

bool TimeProvider::SetCurrentTime(TimePoint pointInTime)
{
    const TimeSpan span = TimePointToTimeSpan(pointInTime);
    if (OS_RESULT_FAILED(System::TakeSemaphore(timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return false;
    }

    CurrentTime = span;
    NotificationTime = TimeSpanAdd(NotificationPeriod, TimeSpanFromMilliseconds(1));
    PersistanceTime = TimeSpanAdd(SavePeriod, TimeSpanFromMilliseconds(1));
    struct TimerState state = BuildTimerState();
    System::GiveSemaphore(timerLock);

    ProcessChange(state);
    return true;
}

Option<TimeSpan> TimeProvider::GetCurrentTime()
{
    if (OS_RESULT_FAILED(System::TakeSemaphore(timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return None<TimeSpan>();
    }

    TimeSpan currentTime = CurrentTime;

    System::GiveSemaphore(timerLock);
    return Some(currentTime);
}

Option<TimePoint> TimeProvider::GetCurrentMissionTime()
{
    const Option<TimeSpan> result = GetCurrentTime();
    if (result.HasValue)
    {
        return Some(TimePointFromTimeSpan(result.Value));
    }

    return None<TimePoint>();
}

void TimeProvider::ProcessChange(TimerState state)
{
    if (OS_RESULT_FAILED(System::TakeSemaphore(notificationLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire notification lock.");
        return;
    }

    SendTimeNotification(state);
    SaveTime(state);

    System::GiveSemaphore(notificationLock);
}

TimerState TimeProvider::BuildTimerState()
{
    struct TimerState result;
    result.time = CurrentTime;
    result.saveTime = TimeSpanLessThan(SavePeriod, PersistanceTime);
    result.sendNotification = TimeSpanLessThan(NotificationPeriod, NotificationTime);
    if (result.saveTime)
    {
        PersistanceTime = TimeSpanFromMilliseconds(0ull);
    }

    if (result.sendNotification)
    {
        NotificationTime = TimeSpanFromMilliseconds(0ull);
    }

    return result;
}

struct TimeSnapshot TimeProvider::ReadFile(FileSystem* fs, const char* const filePath)
{
    struct TimeSnapshot result = {{0}};
    uint8_t buffer[sizeof(TimeSpan)];
    if (!FileSystemReadFile(fs, filePath, buffer, sizeof(buffer)))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s.", filePath);
        return result;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, sizeof(buffer));
    result.CurrentTime.value = ReaderReadQuadWordLE(&reader);
    if (!ReaderStatus(&reader))
    {
        LOGF(LOG_LEVEL_WARNING, "Not enough data read from file: %s. ", filePath);
        return result;
    }

    return result;
}

struct TimeSnapshot TimeProvider::CurrentPersistentTime(FileSystem* fileSystem)
{
    if (fileSystem == NULL)
    {
        struct TimeSnapshot snapshot = {{0}};
        return snapshot;
    }

    struct TimeSnapshot snapshot[3];
    snapshot[0] = ReadFile(fileSystem, File0);
    snapshot[1] = ReadFile(fileSystem, File1);
    snapshot[2] = ReadFile(fileSystem, File2);

    // every value has one initial copy (its own).
    int votes[3] = {1, 1, 1};

    // now vote to determine the most common value,
    // by increasing counters that are the same.
    if (snapshot[0] == snapshot[1])
    {
        ++votes[0];
        ++votes[1];
    }

    if (snapshot[0] == snapshot[2])
    {
        ++votes[0];
        ++votes[2];
    }
    // we can get away with else here since we seek only one maximum value not all of them
    // we also do not need exact order
    else if (snapshot[1] == snapshot[2])
    {
        ++votes[1];
        ++votes[2];
    }

    // now that we have voted find the index with the highest count
    int selectedIndex = 0;
    if (votes[1] > votes[0])
    {
        selectedIndex = 1;
    }

    if (votes[2] > votes[selectedIndex])
    {
        selectedIndex = 2;
    }

    // all of the values are different so pick the smallest one as being safest....
    if (votes[selectedIndex] == 1)
    {
        // find the index with the oldest time snapshot.
        selectedIndex = 0;
        if (snapshot[1] < snapshot[0])
        {
            selectedIndex = 1;
        }

        if (snapshot[2] < snapshot[selectedIndex])
        {
            selectedIndex = 2;
        }
    }

    return snapshot[selectedIndex];
}

void TimeProvider::SendTimeNotification(TimerState state)
{
    if (state.sendNotification && OnTimePassed != NULL)
    {
        OnTimePassed(TimePassedCallbackContext, TimePointFromTimeSpan(state.time));
    }

    if (state.sendNotification)
    {
        System::PulseSet(TickNotification);
    }
}

void TimeProvider::SaveTime(TimerState state)
{
    if (                         //
        !state.saveTime ||       //
        FileSystemObject == NULL //
        )
    {
        return;
    }

    uint8_t buffer[sizeof(TimeSpan)];
    Writer writer;

    WriterInitialize(&writer, buffer, sizeof(buffer));
    WriterWriteQuadWordLE(&writer, state.time.value);
    if (!WriterStatus(&writer))
    {
        return;
    }

    const uint16_t length = WriterGetDataLength(&writer);
    int retryCounter = 0;
    int errorCount = 0;
    int totalErrorCount = 0;
    do
    {
        errorCount = 0;
        if (!FileSystemSaveToFile(FileSystemObject, File0, buffer, length))
        {
            ++errorCount;
        }
        if (!FileSystemSaveToFile(FileSystemObject, File1, buffer, length))
        {
            ++errorCount;
        }
        if (!FileSystemSaveToFile(FileSystemObject, File2, buffer, length))
        {
            ++errorCount;
        }

        totalErrorCount += errorCount;
    } while (++retryCounter < 3 && errorCount > 1);

    if (totalErrorCount > 0)
    {
        LOGF(LOG_LEVEL_WARNING, "[timer] Timer encountered %d errors over %d state save attempts. ", totalErrorCount, errorCount);
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

        if (OS_RESULT_FAILED(System::PulseWait(TickNotification, MAX_DELAY)))
        {
            return false;
        }
    } while (true);

    return false;
}

bool TimeProvider::LongDelay(TimeSpan delay)
{
    Option<TimePoint> missionTime = GetCurrentMissionTime();

    if (!missionTime.HasValue)
    {
        return false;
    }

    TimePoint time = TimePointFromTimeSpan(TimeSpanAdd(TimePointToTimeSpan(missionTime.Value), delay));

    return LongDelayUntil(time);
}

/** @} */
