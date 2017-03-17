#ifndef LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_
#define LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_

#pragma once

#include "mission/base.hpp"
#include "rtc/rtc.hpp"
#include "state/struct.h"
#include "time/timer.h"

#include <tuple>

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
     * @brief Task that is responsible for updating current state in global mission state object and correct time based on external RTC.
     */
    class TimeTask : public Update, public Action
    {
      public:
        /**
         * @brief ctor.
         * @param[in] arguments Reference to time providier argument list.
         */
        TimeTask(std::tuple<TimeProvider&, devices::rtc::IRTC&> arguments);

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
        static void CorrectTime(const SystemState& state, void* param);

        /**
         * @brief Time provider reference.
         */
        TimeProvider& provider;

        /**
         * @brief External RTC reference.
         */
        devices::rtc::IRTC& rtc;

        /**
         * @brief Time of last correction.
         */
        Option<std::chrono::milliseconds> lastTimeCorrection;

        /**
         * @brief Last seen value of internal mission time.
         */
        Option<std::chrono::milliseconds> lastMissionTime;

        /**
         * @brief Last seen value of external RTC.
         */
        Option<std::chrono::seconds> lastExternalClockTime;

        /**
         * @brief Procedure that reads initial clock values.
         * @param[in] time Reference current mission time.
         */
        void ReadInitialClockValues(const Option<std::chrono::milliseconds>& time);

        /**
         * @brief Procedure corrects mission time based on external RTC.
         * @param[in] time Reference current mission time.
         */
        void CorrectTime(const Option<std::chrono::milliseconds>& time);
    };

    /** @brief Minimal period of time correction. */
    static constexpr std::chrono::milliseconds TimeCorrectionPeriod = std::chrono::minutes(15);

    /** @brief Warning level of time correction delta after which warning will be logged. */
    static constexpr std::chrono::milliseconds TimeCorrectionWarningThreshold = std::chrono::seconds(15);

    /** @brief Minimum value of time correction delta to update the time in time provider. */
    static constexpr std::chrono::milliseconds MinimumTimeCorrection = std::chrono::seconds(2);

    /** @} */
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_ */
