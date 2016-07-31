#include "timer.h"
#include "base/reader.h"
#include "base/writer.h"
#include "logger/logger.h"

static const char* const TimeFile0 = "TimeState.0";
static const char* const TimeFile1 = "TimeState.1";
static const char* const TimeFile2 = "TimeState.2";

static const TimeSpan NotificationPeriod = 10000; // 10s

static const TimeSpan SavePeriod = 15 * 60 * 1000; // 15 min

static void TimeTickProcedure(struct TimeProvider* provider, TimeSpan delta);

static void SendTimeNotification(struct TimeProvider* provider);

static void SaveTime(struct TimeProvider* provider);

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
    struct TimeSnapshot snapshot = {0};
    if (fileSystem != NULL)
    {
        struct TimeSnapshot snapshot0 = ReadFile(fileSystem, TimeFile0);
        struct TimeSnapshot snapshot1 = ReadFile(fileSystem, TimeFile1);
        struct TimeSnapshot snapshot2 = ReadFile(fileSystem, TimeFile2);
        // TODO
    }

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

static void SaveTime(struct TimeProvider* timeProvider)
{
    if (timeProvider->PersistanceTime <= SavePeriod)
    {
        return;
    }

    if (timeProvider->FileSystemObject != NULL)
    {
        uint8_t buffer[sizeof(TimeSpan)];
        Writer writer;

        WriterInitialize(&writer, buffer, sizeof(buffer));
        WriterWriteQuadWordLE(&writer, timeProvider->CurrentTime);
        if (!WriterStatus(&writer))
        {
            return;
        }

        const uint16_t length = WriterGetDataLength(&writer);
        SaveToFile(timeProvider->FileSystemObject, TimeFile0, buffer, length);
        SaveToFile(timeProvider->FileSystemObject, TimeFile1, buffer, length);
        SaveToFile(timeProvider->FileSystemObject, TimeFile2, buffer, length);
    }

    timeProvider->PersistanceTime = 0;
}
