#include "timer.h"
#include <stdlib.h>
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"

static const char* const TimeFile0 = "TimeState.0";
static const char* const TimeFile1 = "TimeState.1";
static const char* const TimeFile2 = "TimeState.2";

static const TimeSpan NotificationPeriod = 10000; // 10s

static const TimeSpan SavePeriod = 15 * 60 * 1000; // 15 min

struct TimerState
{
    TimeSpan time;

    bool saveTime;

    bool sendNotification;
};

static void TimeTickProcedure(struct TimeProvider* provider, TimeSpan delta);

static void SendTimeNotification(struct TimeProvider* provider, struct TimerState state);

static void SaveTime(struct TimeProvider* provider, struct TimerState state);

static struct TimerState TimeBuildTimerState(struct TimeProvider* TimeProvider);

static void TimeProcessChange(struct TimeProvider* timeProvider, struct TimerState state);

bool TimeInitialize(
    struct TimeProvider* provider, TimePassedCallbackType timePassedCallback, void* timePassedCallbackContext, FileSystem* fileSystem)
{
    const struct TimeSnapshot snapshot = GetCurrentPersistentTime(fileSystem);
    provider->CurrentTime = snapshot.CurrentTime;
    provider->NotificationTime = 0;
    provider->PersistanceTime = 0;
    provider->OnTimePassed = timePassedCallback;
    provider->TimePassedCallbackContext = timePassedCallbackContext;
    provider->FileSystemObject = fileSystem;
    provider->timerLock = System.CreateBinarySemaphore();
    provider->notificationLock = System.CreateBinarySemaphore();
    const bool result = provider->timerLock != NULL && provider->notificationLock != NULL;
    return result;
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

bool TimeSetCurrentTime(struct TimeProvider* timeProvider, TimePoint pointInTime)
{
    const TimeSpan span = TimePointToTimeSpan(pointInTime);
    if (OS_RESULT_FAILED(System.TakeSemaphore(timeProvider->timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return false;
    }

    timeProvider->CurrentTime = span;
    timeProvider->NotificationTime = NotificationPeriod + 1;
    timeProvider->PersistanceTime = SavePeriod + 1;
    struct TimerState state = TimeBuildTimerState(timeProvider);
    System.GiveSemaphore(timeProvider->timerLock);

    TimeProcessChange(timeProvider, state);
    return true;
}

TimeSpan TimeGetCurrentTime(struct TimeProvider* timeProvider)
{
    return timeProvider->CurrentTime;
}

TimePoint TimeGetCurrentMissionTime(struct TimeProvider* timeProvider)
{
    return TimePointFromTimeSpan(TimeGetCurrentTime(timeProvider));
}

void TimeTickProcedure(struct TimeProvider* timeProvider, TimeSpan delta)
{
    if (OS_RESULT_FAILED(System.TakeSemaphore(timeProvider->timerLock, MAX_DELAY)))
    {
        LOG(LOG_LEVEL_ERROR, "Unable to acquire timer lock.");
        return;
    }

    timeProvider->CurrentTime += delta;
    timeProvider->NotificationTime += delta;
    timeProvider->PersistanceTime += delta;

    struct TimerState state = TimeBuildTimerState(timeProvider);

    System.GiveSemaphore(timeProvider->timerLock);
    TimeProcessChange(timeProvider, state);
}

void TimeProcessChange(struct TimeProvider* timeProvider, struct TimerState state)
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

struct TimerState TimeBuildTimerState(struct TimeProvider* timeProvider)
{
    struct TimerState result;
    result.time = timeProvider->CurrentTime;
    result.saveTime = timeProvider->PersistanceTime >= SavePeriod;
    result.sendNotification = timeProvider->NotificationTime >= NotificationPeriod;
    if (result.saveTime)
    {
        timeProvider->PersistanceTime = 0;
    }

    if (result.sendNotification)
    {
        timeProvider->NotificationTime = 0;
    }

    return result;
}

static struct TimeSnapshot ReadFile(FileSystem* fs, const char* const filePath)
{
    struct TimeSnapshot result = {0};
    uint8_t buffer[sizeof(TimeSpan)];
    const FSFileHandle handle = fs->open(filePath, O_RDONLY, S_IRWXU);
    if (handle == -1)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", filePath);
        return result;
    }

    const int read = fs->read(handle, buffer, sizeof(buffer));
    fs->close(handle);
    if (read != sizeof(buffer))
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to read file: %s. Status: 0x%08x", filePath, fs->getLastError());
        return result;
    }

    Reader reader;
    ReaderInitialize(&reader, buffer, read);
    result.CurrentTime = ReaderReadQuadWordLE(&reader);
    if (!ReaderStatus(&reader))
    {
        LOGF(LOG_LEVEL_WARNING, "Not enough date read from file: %s. Length: 0x%08x", filePath, read);
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
    int counters[3] = {1, 1, 1};

    // now vote to determine the most common value,
    // by increasing counters that are the same.
    if (TimeSnapshotEqual(snapshot[0], snapshot[1]))
    {
        ++counters[0];
        ++counters[1];
    }

    if (TimeSnapshotEqual(snapshot[0], snapshot[2]))
    {
        ++counters[0];
        ++counters[2];
    }
    // we can get away with else here since we seek only one maximum value not all of them
    // we also do not need exact order
    else if (TimeSnapshotEqual(snapshot[1], snapshot[2]))
    {
        ++counters[1];
        ++counters[2];
    }

    // now that we have voted find the index with the highest count
    int selectedIndex = 0;
    if (counters[1] > counters[0])
    {
        selectedIndex = 1;
    }

    if (counters[2] > counters[selectedIndex])
    {
        selectedIndex = 2;
    }

    // all of the values are different so pick the smallest one as being safest....
    if (counters[selectedIndex] == 1)
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

static void SaveToFile(FileSystem* fs, const char* file, const uint8_t* buffer, uint16_t length)
{
    const FSFileHandle handle = fs->open(file, O_WRONLY | O_CREAT, S_IRWXU);
    if (handle == -1)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to open file: %s", file);
        return;
    }

    const int result = fs->write(handle, buffer, length);
    if (result != length)
    {
        LOGF(LOG_LEVEL_WARNING, "Unable to update file: %s. Status: 0x%08x", file, fs->getLastError());
    }

    fs->close(handle);
}

static void SendTimeNotification(struct TimeProvider* timeProvider, struct TimerState state)
{
    if (state.sendNotification)
    {
        timeProvider->OnTimePassed(timeProvider->TimePassedCallbackContext, TimePointFromTimeSpan(state.time));
    }
}

static void SaveTime(struct TimeProvider* timeProvider, struct TimerState state)
{
    if (state.saveTime && timeProvider->FileSystemObject != NULL)
    {
        uint8_t buffer[sizeof(TimeSpan)];
        Writer writer;

        WriterInitialize(&writer, buffer, sizeof(buffer));
        WriterWriteQuadWordLE(&writer, state.time);
        if (!WriterStatus(&writer))
        {
            return;
        }

        const uint16_t length = WriterGetDataLength(&writer);
        SaveToFile(timeProvider->FileSystemObject, TimeFile0, buffer, length);
        SaveToFile(timeProvider->FileSystemObject, TimeFile1, buffer, length);
        SaveToFile(timeProvider->FileSystemObject, TimeFile2, buffer, length);
    }
}
