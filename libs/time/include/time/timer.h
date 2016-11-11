#ifndef SRC_TIMER_H
#define SRC_TIMER_H

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "TimePoint.h"
#include "base/os.h"
#include "fs/fs.h"
#include "system.h"

/**
 * @defgroup time Time abstraction
 *
 * @brief This module contains timer object that is responsible for measuring absolute mission time in milliseconds.
 * @{
 */

EXTERNC_BEGIN

struct TimeProviderTag;

/**
 * @brief Type definition of the rtc notification routine.
 * @param[in] provider Pointer to timer provider that should update its internal state.
 * @param[in] delta The amount of time that has passed since last timer notification.
 */
typedef void (*TimeTickCallbackType)(struct TimeProviderTag* provider, TimeSpan delta);

/**
 * @brief Type definition of the callback procedure called on time change notification.
 * @param[in] context Time notification callback context. This is a copy of the
 * timePassedCallbackContext parameter passed to TimeInitialize procedure.
 * @param[in] currentTime Current mission time in milliseconds.
 */
typedef void (*TimePassedCallbackType)(void* context, TimePoint currentTime);

/**
 * @brief This structure represents the contents of the time snapshot that gets
 * persisted on in file.
 */
struct TimeSnapshot
{
    /**
     * @brief Current mission time in milliseconds.
     */
    TimeSpan CurrentTime;
};

/**
 * @brief Monotonic persistent timer.
 *
 * This type represents timer that is responsible for measuring absolute mission time in milliseconds.
 * This timer does not utilizes any hardware it should be registered in the dedicated timer that will periodically notify it
 * about time change with the time delta in ms.
 *
 * This timer saves the current mission time once every 15 minutes. The time change notification is sent once every 10s.
 *
 * The current time is saved to three files to reduce the possibility of corruption. If there are different time
 * values in those files or any of those files is not available the majority vote is done to determine the most
 * likely correct value. In case when all of the values are different the smallest one is selected as the correct one.
 */
typedef struct TimeProviderTag
{
    /**
     * @brief Pointer to time notification procedure that gets called on time change.
     * @see TimePassedCallbackType for details.
     */
    TimePassedCallbackType OnTimePassed;

    /**
     * @brief Time notification procedure context pointer.
     */
    void* TimePassedCallbackContext;

    /**
     * @brief Current mission time in milliseconds.
     *
     * This value is protected by the timerLock semaphore.
     */
    TimeSpan CurrentTime;

    /**
     * @brief Time period since last timer notification.
     *
     * This value is used to determine whether the time notification should be invoked on next rtc notification.
     * This value is protected by the timerLock semaphore.
     */
    TimeSpan NotificationTime;

    /**
     * @brief Time period since the last timer state save.
     *
     * This value is used to determine whether the time state should be saved on next rtc notification.
     * This value is protected by the timerLock semaphore.
     */
    TimeSpan PersistanceTime;

    /**
     * @brief Semaphore used to protect internal timer state.
     *
     * This value is used to synchronize access to current mission time.
     */
    OSSemaphoreHandle timerLock;

    /**
     * @brief Semaphore used to ensure that only one time notification callback is being executed at any given time.
     */
    OSSemaphoreHandle notificationLock;

    /**
     * @brief Pulse notifed on each timer tick
     */
    OSPulseHandle TickNotification;

    /**
     * @brief Pointer to file system object that is used to save/restore timer state.
     */
    FileSystem* FileSystemObject;
} TimeProvider;

/**
 * @brief Initializes the timer object.
 *
 * @param[out] provider Timer object that should be initialized.
 * @param[in] timePassedCallback Pointer to time notification routine that should be called
 * on mission time change.
 * @param[in] timePassedCallbackContext timePassedCallback context pointer. This value is not used by the timer itself.
 * It passes it as the context parameter to the timePassedCallback.
 * @param[in] fileSystem Pointer to file system object that should be used to read persistent state.
 * @return Operation status. True on success, false otherwise.
 *
 * Besides the time initialization this procedure will automatically restores the timer state from the persistent state
 * saved in the files on local flash memory, therefore make sure that file system module is already initialized.
 */
bool TimeInitialize(TimeProvider* provider,    //
    TimePassedCallbackType timePassedCallback, //
    void* timePassedCallbackContext,           //
    FileSystem* fileSystem                     //
    );

/**
 * @brief This procedure returns current mission time in milliseconds.
 *
 * @param[in] timeProvider Pointer to queried timer object.
 * @param[out] currentTime Pointer to time span value that should be
 * updated with current mission time.
 * @return True on success, false otherwise.
 */
bool TimeGetCurrentTime(TimeProvider* timeProvider, TimeSpan* currentTime);

/**
 * @brief This procedure returns current mission time in decoded format.
 *
 * @param[in] timeProvider Pointer to queried timer object.
 * @param[out] timePoint Pointer to time point value that should be
 * updated with current decoded mission time.
 * @return True on success, false otherwise.
 */
bool TimeGetCurrentMissionTime(TimeProvider* timeProvider, TimePoint* timePoint);

/**
 * @brief This procedure can be used to move timer state forward by specified amount.
 *
 * Be aware that moving time forward far enough may trigger timer notification &
 * timer state save process therefore the procedure may be take some time to complete.
 * @param[in] timeProvider Pointer to timer object whose state should be updated.
 * @param[in] delta The amount of time that timer state should be moved forward.
 */
void TimeAdvanceTime(TimeProvider* timeProvider, TimeSpan delta);

/**
 * @brief This procedure sets the current mission time to any arbitrary point in time.
 *
 * The currently saved time gets immediately preserved and propagated to the notification routine.
 * @param[in] timeProvider Pointer to timer object whose state should be updated.
 * @param[in] pointInTime New timer state.
 *
 * @return Operation status. True on success, false otherwise.
 */
bool TimeSetCurrentTime(TimeProvider* timeProvider, TimePoint pointInTime);

/**
 * @brief Returns timer rtc notification routine.
 *
 * This procedure returned by this function should not be called directly from the
 * interrupt service routine.
 *
 * @return Pointer to timer rtc notification routine.
 */
TimeTickCallbackType TimeGetTickProcedure(void);

/**
 * @brief This procedure is responsible for reading the last timer state that has been
 * preserved in the persistent memory.
 *
 * @param[in] fileSystem Pointer to file system object that should be used to read persistent state.
 * @return Either last stable timer state that get read from the persistent memory or
 * value indicating zero (initial time).
 */
struct TimeSnapshot GetCurrentPersistentTime(FileSystem* fileSystem);

/**
 * @brief This function compares two time snapshots and determines whether two passed time snapshots
 * are considered equal.
 * the first one
 * represents time that is considered 'smaller" than the second once.
 *
 * @param[in] left Time snapshot to compare.
 * @param[in] right Time snapshot to compare.
 * @return True if both time snapshots are considered equal, false otherwise.
 */
static inline bool TimeSnapshotEqual(struct TimeSnapshot left, struct TimeSnapshot right)
{
    return TimeSpanEqual(left.CurrentTime, right.CurrentTime);
}

/**
 * @brief This function compares two time snapshots and determines whether the first one
 * represents time that is considered 'smaller" than the second once.
 *
 * @param[in] left Time snapshot to compare.
 * @param[in] right Time snapshot to compare.
 * @return True if left time snapshot is considered smaller than the right one, false otherwise.
 */
static inline bool TimeSnapshotLessThan(struct TimeSnapshot left, struct TimeSnapshot right)
{
    return TimeSpanLessThan(left.CurrentTime, right.CurrentTime);
}

/**
 * @brief Waits until given timepoint. Wait is directed by timer notifications
 * @param[in] timeProvider Time provider
 * @param[in] time Wanted time point
 * @return True if expected time point is reached, false in case of error
 */
bool TimeLongDelayUntil(TimeProvider* timeProvider, TimePoint time);

/**
 * @brief Waits for given time span. Wait is directed by timer notifications
 * @param[in] timeProvider Time provider
 * @param[in] delay Time span to wait
 * @return True if expected time span is elapsed, false in case of error
 */
bool TimeLongDelay(TimeProvider* timeProvider, TimeSpan delay);

/** @}*/

EXTERNC_END

#endif
