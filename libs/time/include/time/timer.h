#ifndef SRC_TIMER_H
#define SRC_TIMER_H

#pragma once

#include "TimePoint.h"
#include "base/os.h"
#include "fs/fs.h"
#include "utils.h"

#ifndef TIMER_NOTIFICATION_PERIOD
#define TIMER_NOTIFICATION_PERIOD 5000
#endif

#ifndef TIMER_SAVE_PERIOD
#define TIMER_SAVE_PERIOD (15 * 60 * 1000)
#endif

namespace services
{
    namespace time
    {
        /**
         * @defgroup time Time abstraction
         *
         * @brief This module contains timer object that is responsible for measuring absolute mission time in milliseconds.
         * @{
         */

        /**
         * @brief Type definition of the callback procedure called on time change notification.
         * @param[in] context Time notification callback context. This is a copy of the
         * timePassedCallbackContext parameter passed to TimeInitialize procedure.
         * @param[in] currentTime Current mission time in milliseconds.
         */
        using TimePassedCallbackType = void (*)(void* context, TimePoint currentTime);

        /**
         * @brief This structure contains temporary timer state used for passing information between
         * timer components during single rtc time notification.
         */
        struct TimerState
        {
            /**
             * @brief Current mission time in milliseconds.
             */
            std::chrono::milliseconds time;

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
         * @brief This structure represents the contents of the time snapshot that gets
         * persisted on in file.
         */
        struct TimeSnapshot
        {
            /**
             * @brief Current mission time in milliseconds.
             */
            std::chrono::milliseconds CurrentTime;

            TimeSnapshot() : CurrentTime(0ull)
            {
            }

            bool operator==(const TimeSnapshot& right) const
            {
                return CurrentTime == right.CurrentTime;
            }

            bool operator!=(const TimeSnapshot& right) const
            {
                return !(*this == right);
            }

            bool operator<(const TimeSnapshot& right) const
            {
                return CurrentTime < right.CurrentTime;
            }

            bool operator>(const TimeSnapshot& right) const
            {
                return right < *this;
            }

            bool operator<=(const TimeSnapshot& right) const
            {
                return !(*this > right);
            }

            bool operator>=(const TimeSnapshot& right) const
            {
                return !(*this < right);
            }
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
        class TimeProvider
        {
          public:
            /**
             *  @brief Constructor for timer object.
             *
             *  @param[in] fileSystem Pointer to file system object that should be used to read persistent state.
             */
            TimeProvider(services::fs::IFileSystem& fileSystem);

            /**
             * @brief Initializes the timer object.
             *
             * @param[in] timePassedCallback Pointer to time notification routine that should be called
             * on mission time change.
             * @param[in] timePassedCallbackContext timePassedCallback context pointer. This value is not used by the timer itself.
             * It passes it as the context parameter to the timePassedCallback.
             * @return Operation status. True on success, false otherwise.
             *
             * Besides the time initialization this procedure will automatically restores the timer state from the persistent state
             * saved in the files on local flash memory, therefore make sure that file system module is already initialized.
             */
            bool Initialize(TimePassedCallbackType timePassedCallback, //
                void* timePassedCallbackContext                        //
                );

            /**
             * @brief This procedure returns current mission time in milliseconds.
             *
             * @return Option containing current mission time on success, empty option otherwise.
             */
            Option<std::chrono::milliseconds> GetCurrentTime();

            /**
             * @brief This procedure returns current mission time in decoded format.
             *
             * @return Option containing current mission time on success, empty option otherwise.
             */
            Option<TimePoint> GetCurrentMissionTime();

            /**
             * @brief This procedure can be used to move timer state forward by specified amount.
             *
             * Be aware that moving time forward far enough may trigger timer notification &
             * timer state save process therefore the procedure may be take some time to complete.
             * @param[in] delta The amount of time that timer state should be moved forward.
             */
            void AdvanceTime(std::chrono::milliseconds delta);

            /**
             * @brief This procedure sets the current mission time to any arbitrary point in time.
             *
             * The currently saved time gets immediately preserved and propagated to the notification routine.
             * @param[in] pointInTime New timer state.
             *
             * @return Operation status. True on success, false otherwise.
             */
            bool SetCurrentTime(TimePoint pointInTime);

            /**
             * @brief This procedure is responsible for reading the last timer state that has been
             * preserved in the persistent memory.
             *
             * @param[in] fileSystem Reference to the file system sub system that should be used for data access.
             * @return Either last stable timer state that get read from the persistent memory or
             * value indicating zero (initial time).
             */
            struct TimeSnapshot CurrentPersistentTime(services::fs::IFileSystem& fileSystem);

            /**
             * @brief Waits until given timepoint. Wait is directed by timer notifications
             * @param[in] time Wanted time point
             * @return True if expected time point is reached, false in case of error
             */
            bool LongDelayUntil(TimePoint time);

            /**
             * @brief Waits for given time span. Wait is directed by timer notifications
             * @param[in] delay Time span to wait
             * @return True if expected time span is elapsed, false in case of error
             */
            bool LongDelay(std::chrono::milliseconds delay);

          public:
            /**
             * @brief ID used to obtain the timer lock.
             */
            static constexpr uint8_t TIMER_LOCK_ID = 1;

            /**
             * @brief ID used to obtain the notification lock.
             */
            static constexpr uint8_t NOTIFICATION_LOCK_ID = 2;

          private:
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
             * @brief Current mission time in milliseconds.
             *
             * This value is protected by the timerLock semaphore.
             */
            std::chrono::milliseconds CurrentTime;

          private:
            /**
             * @brief This procedure is responsible for saving specified timer state.
             *
             * This method does not determine whether or not state should be saved, for that it
             * uses information from the state object that should be prepared in advance by the caller
             * to avoid excessive timer locking in order to obtain stable timer state. The value
             * that gets saved comes from the state object not the timer itself.
             *
             * @param[in] state The state of the timer object that should be saved.
             */
            void SaveTime(TimerState state);

            /**
             * @brief This method is responsible for rtc timer notification post processing.
             *
             * It is responsible for coordinating the sending passing time notification and
             * saving the timer state. This method ensures that there is only on timer
             * notification being executed for the specified timer object at any given time
             * and similarly that there is at most one task that saves current timer state.
             * @param[in] state Captured timer state that is valid for processing.
             */
            void ProcessChange(TimerState state);

            /**
             * @brief This procedure is responsible for sending passing time notifications to timer clients.
             *
             * This procedure does not determine whether notification should be
             * sent or not, for that it uses the information passed in the state object. The state object
             * should be prepared by the caller to avoid excessive timer locking in order to obtain stable
             * timer state.
             * @param[in] state Timer object state snapshot.
             */
            void SendTimeNotification(TimerState state);

            /**
             * @brief This method generates time state snapshot.
             *
             * In addition this method also determines whether the time notification should be sent
             * immediately as well as whether the timer state should also be saved and updates the
             * timer state accordingly.
             *
             * @return Captured timer state snapshot.
             * @see TimerState
             */
            TimerState BuildTimerState();

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
            struct TimeSnapshot ReadFile(services::fs::IFileSystem& fs, const char* const filePath);

          private:
            static constexpr const char* File0 = "/TimeState.0";
            static constexpr const char* File1 = "/TimeState.1";
            static constexpr const char* File2 = "/TimeState.2";

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
             * @brief Time period since last timer notification.
             *
             * This value is used to determine whether the time notification should be invoked on next rtc notification.
             * This value is protected by the timerLock semaphore.
             */
            std::chrono::milliseconds NotificationTime;

            /**
             * @brief Time period since the last timer state save.
             *
             * This value is used to determine whether the time state should be saved on next rtc notification.
             * This value is protected by the timerLock semaphore.
             */
            std::chrono::milliseconds PersistanceTime;

            /**
             * @brief Pulse notified on each timer tick
             */
            OSPulseHandle TickNotification;

            /**
             * @brief File system object that is used to save/restore timer state.
             */
            services::fs::IFileSystem& FileSystemObject;
        };

        /** @}*/
    }
}

#endif
