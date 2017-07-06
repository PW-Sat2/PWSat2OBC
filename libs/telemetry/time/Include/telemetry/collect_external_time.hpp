#ifndef LIBS_TELEMETRY_COLLECT_EXTERNAL_TIME_HPP
#define LIBS_TELEMETRY_COLLECT_EXTERNAL_TIME_HPP

#pragma once

#include "mission/base.hpp"
#include "rtc/rtc.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating external timer telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class ExternalTimeTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] rtcDriver Reference to antenna driver that will provide this module with hardware telemetry
         */
        ExternalTimeTelemetryAcquisition(devices::rtc::IRTC& rtcDriver);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the external timer telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires external timer telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new external timer telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current external timer telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to rtc driver.
         */
        devices::rtc::IRTC* provider;
    };
}

#endif
