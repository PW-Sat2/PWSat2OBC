#ifndef LIBS_TELEMETRY_GYRO_COLLECT_HPP
#define LIBS_TELEMETRY_GYRO_COLLECT_HPP

#pragma once

#include "gyro/gyro.h"
#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating gyroscope hardware telemetry.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class GyroTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] gyro Reference to gyroscope driver that will provide this module with hardware telemetry.
         */
        GyroTelemetryAcquisition(devices::gyro::IGyroscopeDriver& gyro);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the gyroscope telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires gyroscope telemetry & stores it in passed state object.
         * @param[in] state Object that should be updated with new gyroscope hardware telemetry.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateCommTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current gyroscope telemetry in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to gyroscope driver.
         */
        devices::gyro::IGyroscopeDriver* provider;
    };
}

#endif
