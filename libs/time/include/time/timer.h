#ifndef SRC_TIMER_H
#define SRC_TIMER_H

#pragma once

#include "ICurrentTime.hpp"
#include "TimePoint.h"
#include "base/os.h"
#include "fs/fs.h"
#include "utils.h"

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

            /**
             * @brief Equality operator
             * @param right Other value
             * @return true if both values are equal
             */
            bool operator==(const TimeSnapshot& right) const
            {
                return CurrentTime == right.CurrentTime;
            }

            /**
             * @brief Inequality operator
             * @param right Other value
             * @return true if both values are not equal
             */
            bool operator!=(const TimeSnapshot& right) const
            {
                return !(*this == right);
            }

            /**
             * @brief Less then operator
             * @param right Other value
             * @return true if this is less then other
             */
            bool operator<(const TimeSnapshot& right) const
            {
                return CurrentTime < right.CurrentTime;
            }

            /**
             * @brief Greater then operator
             * @param right Other value
             * @return true if this is greater then other
             */
            bool operator>(const TimeSnapshot& right) const
            {
                return right < *this;
            }

            /**
             * @brief Less then or equal operator
             * @param right Other value
             * @return true if this is less then or equal to other
             */
            bool operator<=(const TimeSnapshot& right) const
            {
                return !(*this > right);
            }

            /**
             * @brief Greater then or equal operator
             * @param right Other value
             * @return true if this is greater then or equal to other
             */
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
        class TimeProvider : public TimeAction, public ICurrentTime
        {
          public:
            /**
             *  @brief Constructor for timer object.
             */
            TimeProvider();

            /**
             * @brief Initializes the timer object.
             *
             * @param[in] startTime Initial time.
             * @param[in] timePassedCallback Pointer to time notification routine that should be called
             * on mission time change.
             * @param[in] timePassedCallbackContext timePassedCallback context pointer. This value is not used by the timer itself.
             * It passes it as the context parameter to the timePassedCallback.
             * @return Operation status. True on success, false otherwise.
             *
             * Besides the time initialization this procedure will automatically restores the timer state from the persistent state
             * saved in the files on local flash memory, therefore make sure that file system module is already initialized.
             */
            bool Initialize(std::chrono::milliseconds startTime, //
                TimePassedCallbackType timePassedCallback,       //
                void* timePassedCallbackContext                  //
                );

            /**
             * @brief This procedure returns current mission time in milliseconds.
             *
             * @return Option containing current mission time on success, empty option otherwise.
             */
            virtual Option<std::chrono::milliseconds> GetCurrentTime() override;

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
            virtual bool SetCurrentTime(TimePoint pointInTime) override;

            /**
             * @brief This procedure sets the current mission time to any arbitrary point in time.
             *
             * The currently saved time gets immediately preserved and propagated to the notification routine.
             * @param[in] duration New timer state.
             *
             * @return Operation status. True on success, false otherwise.
             */
            virtual bool SetCurrentTime(std::chrono::milliseconds duration) override;

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

            /**
             * @brief Method that will be called by BURTC.
             * @param[in] interval Interval that passed since last tick
             */
            void virtual Invoke(std::chrono::milliseconds interval) override;

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
            std::chrono::milliseconds currentTime;

            /**
             * @brief Time period since last timer notification.
             *
             * This value is used to determine whether the time notification should be invoked on next rtc notification.
             * This value is protected by the timerLock semaphore.
             */
            std::chrono::milliseconds notificationTime;

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
             * @brief Pulse notified on each timer tick
             */
            OSPulseHandle TickNotification;
        };

        /** @}*/
    }
}

#endif
