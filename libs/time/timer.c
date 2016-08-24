#include "timer.h"
#include <stdlib.h>
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"

static const char* const TimeFile0 = "/TimeState.0";
static const char* const TimeFile1 = "/TimeState.1";
static const char* const TimeFile2 = "/TimeState.2";

#ifndef TIMER_NOTIFICATION_PERIOD
#define TIMER_NOTIFICATION_PERIOD 5000
#endif

#ifndef TIMER_SAVE_PERIOD
#define TIMER_SAVE_PERIOD (15 * 60 * 1000)
#endif

/**
 * @addtogroup time
 * @{
 */

/**
 * @brief Time period between the subsequent mission time notifications.
 */
static const TimeSpan NotificationPeriod = {TIMER_NOTIFICATION_PERIOD};

/**
 * @brief Time period between subsequent timer state saves.
 */
static const TimeSpan SavePeriod = {TIMER_SAVE_PERIOD};

#undef TIMER_NOTIFICATION_PERIOD
#undef TIMER_SAVE_PERIOD

/**
 * @brief This structure contains temporary timer state used for passing information between
 * timer components during single rtc time notification.
 */
struct TimerState
{
    /**
     * @brief Current mission time in milliseconds.
     */
    TimeSpan time;

    /**
     * @brief Flag indicating whether the timer state should be immediately saved.
     */
    bool saveTime;

    /**
     * @brief Flag indicating whether the timer notification should be immediately called.
     */
    bool sendNotification;
};

/**
 * @brief This procedure is responsible for processing rtc time notifications.
 * @param[in] provider Pointer to timer object that should be notified about passing time.
 * @param[in] delta The amount of time that has passes since last time notification.
 */
static void TimeTickProcedure(TimeProvider* provider, TimeSpan delta);

/**
 * @brief This procedure is responsible for sending passing time notifications to timer clients.
 *
 * This procedure does not determine whether notification should be
 * sent or not, for that it uses the information passed in the state object. The state object
 * should be prepared by the caller to avoid excessive timer locking in order to obtain stable
 * timer state.
 * @param[in] provider Timer object whose notification should be published.
 * @param[in] state Timer object state snapshot.
 */
static void SendTimeNotification(TimeProvider* provider, struct TimerState state);

/**
 * @brief This procedure is responsible for saving specified timer state.
 *
 * This method does not determine whether or not state should be saved, for that it
 * uses information from the state object that should be prepared in advance by the caller
 * to avoid excessive timer locking in order to obtain stable timer state. The value
 * that gets saved comes from the state object not the timer itself.
 *
 * @param[in] provider Time object whose state should be saved.
 * @param[in] state The state of the timer object that should be saved.
 */
static void SaveTime(TimeProvider* provider, struct TimerState state);

/**
 * @brief This method generates time state snapshot.
 *
 * In addition this method also determines whether the time notification should be sent
 * immediately as well as whether the timer state should also be saved and updates the
 * timer state accordingly.
 *
 * @param[in] timeProvider Timer object whose snapshot should be generated.
 * @return Captured timer state snapshot.
 * @see TimerState
 */
static struct TimerState TimeBuildTimerState(TimeProvider* timeProvider);

/**
 * @brief This method is responsible for rtc timer notification post processing.
 *
 * It is responsible for coordinating the sending passing time notification and
 * saving the timer state. This method ensures that there is only on timer
 * notification being executed for the specified timer object at any given time
 * and similarly that there is at most one task that saves current timer state.
 * @param[in] timeProvider Timer object whose rtc notification should be processed.
 * @param[in] state Captured timer state that is valid for processing.
 */
static void TimeProcessChange(TimeProvider* timeProvider, struct TimerState state);

bool TimeInitialize(TimeProvider* provider,    //
    TimePassedCallbackType timePassedCallback, //
    void* timePassedCallbackContext,           //
    FileSystem* fileSystem                     //
    )
{
    const struct TimeSnapshot snapshot = GetCurrentPersistentTime(fileSystem);
    provider->CurrentTime = snapshot.CurrentTime;
    provider->NotificationTime = TimeSpanFromMilliseconds(0ull);
    provider->PersistanceTime = TimeSpanFromMilliseconds(0ull);
    provider->OnTimePassed = timePassedCallback;
    provider->TimePassedCallbackContext = timePassedCallbackContext;
    provider->FileSystemObject = fileSystem;
    provider->timerLock = System.CreateBinarySemaphore();
    provider->notificationLock = System.CreateBinarySemaphore();
    if (provider->timerLock != NULL)
    {
        System.GiveSemaphore(provider->timerLock);
    }

    if (provider->notificationLock != NULL)
    {
        System.GiveSemaphore(provider->notificationLock);
    }

    const bool result = provider->timerLock != NULL && provider->notificationLock != NULL;
    return result;
}

TimeTickCallbackType TimeGetTickProcedure(void)
{
    return TimeTickProcedure;
}

void TimeAdvanceTime(TimeProvider* timeProvider, TimeSpan delta)
{
    TimeTickProcedure(timeProvider, delta);
}

bool TimeSetCurrentTime(TimeProvider* timeProvider, TimePoint pointInTime)
{
    const TimeSpan span = TimePointToTimeSpan(pointInTime);
    if (OS_RESULT_FAILED(System.TakeSemaphore(timeProvider->timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return false;
    }

    timeProvider->CurrentTime = span;
    timeProvider->NotificationTime = TimeSpanAdd(NotificationPeriod, TimeSpanFromMilliseconds(1));
    timeProvider->PersistanceTime = TimeSpanAdd(SavePeriod, TimeSpanFromMilliseconds(1));
    struct TimerState state = TimeBuildTimerState(timeProvider);
    System.GiveSemaphore(timeProvider->timerLock);

    TimeProcessChange(timeProvider, state);
    return true;
}

bool TimeGetCurrentTime(TimeProvider* timeProvider, TimeSpan* currentTime)
{
    if (OS_RESULT_FAILED(System.TakeSemaphore(timeProvider->timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return false;
    }

    *currentTime = timeProvider->CurrentTime;

    System.GiveSemaphore(timeProvider->timerLock);
    return true;
}

bool TimeGetCurrentMissionTime(TimeProvider* timeProvider, TimePoint* timePoint)
{
    TimeSpan span;
    const bool result = TimeGetCurrentTime(timeProvider, &span);
    if (result)
    {
        *timePoint = TimePointFromTimeSpan(span);
    }

    return result;
}

void TimeTickProcedure(TimeProvider* timeProvider, TimeSpan delta)
{
    if (OS_RESULT_FAILED(System.TakeSemaphore(timeProvider->timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return;
    }

    timeProvider->CurrentTime = TimeSpanAdd(timeProvider->CurrentTime, delta);
    timeProvider->NotificationTime = TimeSpanAdd(timeProvider->NotificationTime, delta);
    timeProvider->PersistanceTime = TimeSpanAdd(timeProvider->PersistanceTime, delta);

    struct TimerState state = TimeBuildTimerState(timeProvider);

    System.GiveSemaphore(timeProvider->timerLock);
    TimeProcessChange(timeProvider, state);
}

void TimeProcessChange(TimeProvider* timeProvider, struct TimerState state)
{
    if (OS_RESULT_FAILED(System.TakeSemaphore(timeProvider->notificationLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire notification lock.");
        return;
    }

    SendTimeNotification(timeProvider, state);
    SaveTime(timeProvider, state);

    System.GiveSemaphore(timeProvider->notificationLock);
}

struct TimerState TimeBuildTimerState(TimeProvider* timeProvider)
{
    struct TimerState result;
    result.time = timeProvider->CurrentTime;
    result.saveTime = TimeSpanLessThan(SavePeriod, timeProvider->PersistanceTime);
    result.sendNotification = TimeSpanLessThan(NotificationPeriod, timeProvider->NotificationTime);
    if (result.saveTime)
    {
        timeProvider->PersistanceTime = TimeSpanFromMilliseconds(0ull);
    }

    if (result.sendNotification)
    {
        timeProvider->NotificationTime = TimeSpanFromMilliseconds(0ull);
    }

    return result;
}

/**
 * @brief This procedure is responsible for reading timer state from single file.
 *
 * When selected file does not exist or is empty/corrupted this procedure return
 * default state with time state set to zero (beginning of time).
 * @param[in] fs FileSystem interface for accessing files.
 * @param[in] filePath Path to the file that contains timer state.
 *
 * @return Read timer state or default (zero) state in case of errors.
 */
static struct TimeSnapshot ReadFile(FileSystem* fs, const char* const filePath)
{
    struct TimeSnapshot result = {0};
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

struct TimeSnapshot GetCurrentPersistentTime(FileSystem* fileSystem)
{
    if (fileSystem == NULL)
    {
        struct TimeSnapshot snapshot = {0};
        return snapshot;
    }

    struct TimeSnapshot snapshot[3];
    snapshot[0] = ReadFile(fileSystem, TimeFile0);
    snapshot[1] = ReadFile(fileSystem, TimeFile1);
    snapshot[2] = ReadFile(fileSystem, TimeFile2);

    // every value has one initial copy (its own).
    int votes[3] = {1, 1, 1};

    // now vote to determine the most common value,
    // by increasing counters that are the same.
    if (TimeSnapshotEqual(snapshot[0], snapshot[1]))
    {
        ++votes[0];
        ++votes[1];
    }

    if (TimeSnapshotEqual(snapshot[0], snapshot[2]))
    {
        ++votes[0];
        ++votes[2];
    }
    // we can get away with else here since we seek only one maximum value not all of them
    // we also do not need exact order
    else if (TimeSnapshotEqual(snapshot[1], snapshot[2]))
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
        if (TimeSnapshotLessThan(snapshot[1], snapshot[0]))
        {
            selectedIndex = 1;
        }

        if (TimeSnapshotLessThan(snapshot[2], snapshot[selectedIndex]))
        {
            selectedIndex = 2;
        }
    }

    return snapshot[selectedIndex];
}

static void SendTimeNotification(TimeProvider* timeProvider, struct TimerState state)
{
    if (state.sendNotification)
    {
        timeProvider->OnTimePassed(timeProvider->TimePassedCallbackContext, TimePointFromTimeSpan(state.time));
    }
}

static void SaveTime(TimeProvider* timeProvider, struct TimerState state)
{
    if (                                       //
        !state.saveTime ||                     //
        timeProvider->FileSystemObject == NULL //
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
        if (!FileSystemSaveToFile(timeProvider->FileSystemObject, TimeFile0, buffer, length))
        {
            ++errorCount;
        }
        if (!FileSystemSaveToFile(timeProvider->FileSystemObject, TimeFile1, buffer, length))
        {
            ++errorCount;
        }
        if (!FileSystemSaveToFile(timeProvider->FileSystemObject, TimeFile2, buffer, length))
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

/** @} */
