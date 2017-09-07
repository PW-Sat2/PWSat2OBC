#ifndef LIBS_TELEMETRY_COLLECT_OS_HPP
#define LIBS_TELEMETRY_COLLECT_OS_HPP

#pragma once

#include "mission/base.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
    * @brief This task is responsible for acquiring & updating operating system telemetry
    * @telemetry_acquisition
    * @ingroup telemetry
    */
    class SystemTelemetryAcquisition : public mission::Update
    {
      public:
        /**
        * @brief dummy
        * @param value ignored
        */
        SystemTelemetryAcquisition(int value);
        /**
        * @brief Builds update descriptor for this task.
        * @return Update descriptor - the antenna telemetry acquisition update task.
        */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
        * @brief Updates current operating system telemetry in global state.
        * @param[in] state Reference to global state.
        * @param[in] param Current execution context.
        * @return Telemetry acquisition result.
        */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);
    };
}

#endif
