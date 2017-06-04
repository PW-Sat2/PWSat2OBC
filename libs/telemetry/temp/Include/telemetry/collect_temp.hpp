#ifndef LIBS_TELEMETRY_TEMP_COLLECT_HPP
#define LIBS_TELEMETRY_TEMP_COLLECT_HPP

#pragma once

#include "mission/base.hpp"
#include "telemetry/state.hpp"
#include "temp/mcu.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating current mcu temperature
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class McuTempTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param[in] reader Reference to thermometer driver that will provide this module with current temperature
         */
        McuTempTelemetryAcquisition(temp::ITemperatureReader& reader);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the mcu temperature acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires mcu temperature & stores it in passed state object.
         * @param[in] state Object that should be updated with new value.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateMcuTempTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current mcu temperature in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        /**
         * @brief Reference to internal mcu thermometer driver.
         */
        temp::ITemperatureReader* provider;
    };
}

#endif
