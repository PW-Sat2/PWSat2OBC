#ifndef LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_
#define LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_

#pragma once

#include <tuple>
#include "base/os.h"
#include "mission/base.hpp"
#include "rtc/rtc.hpp"
#include "state/struct.h"
#include "time/timer.h"

using services::time::TimeProvider;

namespace mission
{
    /**
     * @defgroup mission_time Time update
     * @ingroup mission
     *
     * Module that contains all mission tasks that are related strictly to time.
     * @{
     */

    /**
     * @brief Interface for time synchronization.
     */
    struct ITimeSynchronization
    {
        /**
         * @brief Locks time object.
         * @param[in] timeout Timeout.
         * @return True when lock was successful.
         */
        virtual bool Lock(std::chrono::milliseconds timeout) = 0;

        /**
         * @brief Unlocks time object.
         */
        virtual void Unlock() = 0;
    };

    /**
     * @brief Task that is responsible for updating current state in global mission state object
     * and correct time based on external RTC.
     * @mission_task
     *
     * The time synchronization process is run once every 15 minutes assuming that synchronization process succeeds.
     * The time synchronization process will adjust current mission time. The process takes as its input:
     * - current mission time: T<sub>mcu</sub>
     * - current external time: T<sub>rtc</sub>
     * - mission time from last synchronization: T<sub>mcu<sub>n - 1</sub></sub>
     * - external time from last synchronization: T<sub>rtc<sub>n - 1</sub></sub>
     * - internal clock synchronization factor: w<sub>mcu</sub>
     * - external clock synchronization factor: w<sub>rtc</sub>
     *
     * The adjustment process is as follows:
     *
     * delta<sub>mcu</sub> = T<sub>mcu</sub> - T<sub>mcu<sub>n - 1</sub></sub>
     *
     * delta<sub>rtc</sub> = T<sub>rtc</sub> - T<sub>rtc<sub>n - 1</sub></sub>
     *
     * Assuming that time should always monotonically increase, any time increment
     * (either delta<sub>mcu</sub>, delta<sub>rtc</sub> or both) that is negative is set to zero,
     * therefore the negative increments are treated as temporary anomalies not a rule. As a result
     * if for some reason one of the time sources will report decreasing time its increment
     * will be ignored and corrected so it will not pose any problems in future. The current mission time
     * is calculated using the following formula:
     *
     * T<sub>mcu<sub>n</sub></sub> = T<sub>mcu<sub>n - 1</sub></sub> + (delta<sub>mcu</sub> * w<sub>mcu</sub> + delta<sub>rtc</sub> *
     * w<sub>rtc</sub>) / (w<sub>mcu</sub> + w<sub>rtc</sub>)
     *
     * T<sub>rtc<sub>n</sub></sub> = T<sub>rtc</sub>
     *
     * In order to prevent long jumps in time caused by (unlikely) random bit flips the time correction value
     * calculated as abs(T<sub>mcu<sub>n</sub></sub> - T<sub>mcu</sub>) cannot be greater than two hours.
     * As the time synchronization should be done once in 15 minutes then the most likely time correction
     * value should be at most in several seconds at most. Under these conditions, in normal circumstances the
     * time correction value should never reach level of hours. If it does then it is more likely result
     * of random error or external interference then the actual time synchronization error.
     * In such case the time adjustment process is ignored and current mission/external time captured for future.
     */
    class TimeTask : public Update, public Action, public ITimeSynchronization
    {
      public:
        /**
         * @brief Minimal period of time correction.
         * @ingroup mission_time
         */
        static constexpr std::chrono::milliseconds TimeCorrectionPeriod = std::chrono::minutes(15);

        /**
         * @brief Warning level of time correction delta after which warning will be logged.
         * @ingroup mission_time
         */
        static constexpr std::chrono::milliseconds TimeCorrectionWarningThreshold = std::chrono::seconds(15);

        /**
         * @brief Maximum allowed value of time correction delta.
         * @ingroup mission_time
         */
        static constexpr std::chrono::milliseconds MaximumTimeCorrection = std::chrono::hours(2);

        /**
         * @brief ctor.
         * @param[in] arguments Reference to time providier argument list.
         */
        TimeTask(std::tuple<TimeProvider&, devices::rtc::IRTC&, INotifyTimeChanged&> arguments);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the time update task.
         */
        UpdateDescriptor<SystemState> BuildUpdate();

        /**
         * @brief Builds action descriptor for this task.
         * @return Action descriptor - the time correction task.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Locks time object.
         * @param[in] timeout Timeout.
         * @return True when lock was successful.
         */
        virtual bool Lock(std::chrono::milliseconds timeout) override;

        /**
         * @brief Unlocks time object.
         */
        virtual void Unlock() override;

        /**
         * @brief This procedure performs actual time correction/synchronization.
         *
         * @param[in] missionTime Currently observed mission time.
         * @param[in] externalTime Currently observed externally measured time.
         * @param[in] synchronizationState Previous synchronization state.
         * @param[in] correctionConfiguation Time synchronization configuration object.
         * @return Corrected mission time.
         */
        static std::chrono::milliseconds PerformTimeCorrection(std::chrono::milliseconds missionTime, //
            std::chrono::milliseconds externalTime,                                                   //
            const state::TimeState& synchronizationState,                                             //
            const state::TimeCorrectionConfiguration& correctionConfiguation                          //
            );

      private:
        /**
         * @brief Updates current time in global mission state.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static UpdateResult UpdateProc(SystemState& state, void* param);

        /**
         * @brief Condition for time correction action.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         * @return true if correction action should be executed, false otherwise.
         */
        static bool CorrectTimeCondition(const SystemState& state, void* param);

        /**
         * @brief Time correction action, that will correct current time based on the external RTC.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static void CorrectTimeProxy(SystemState& state, void* param);

        /**
         * @brief Time correction procedure, that will correct current time based on the external RTC.
         * @param[in] state Reference to global mission state.
         */
        void CorrectTime(SystemState& state);

        /**
         * @brief Time provider reference.
         */
        TimeProvider& provider;

        /**
         * @brief External RTC reference.
         */
        devices::rtc::IRTC& rtc;

        /**
         * @brief Semaphore used for task synchronization.
         */
        OSSemaphoreHandle syncSemaphore;

        INotifyTimeChanged& _missionLoop;
    };

    /** @} */
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_ */
