#ifndef LIBS_MISSION_INCLUDE_MISSION_TELEMETRY_HPP_
#define LIBS_MISSION_INCLUDE_MISSION_TELEMETRY_HPP_

#pragma once

#include <cstdint>
#include <tuple>
#include "fs/fs.h"
#include "gsl/span"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    /**
     * @defgroup mission_telemetry Telemetry serialization
     * @ingroup mission
     *
     * Module that saves current telemetry state to files.
     * @{
     */

    struct TelemetryConfiguration
    {
        const char* currentFileName;
        const char* previousFileName;
        std::int32_t maxFileSize;
    };
    /**
     * @brief Task that is responsible for updating current state in global mission state object
     * and correct time based on external RTC.
     */
    class TelemetryTask : public Action
    {
      public:
        /**
         * @brief ctor.
         * @param[in] arguments Reference to time providier argument list.
         */
        TelemetryTask(std::tuple<services::fs::IFileSystem&, TelemetryConfiguration> arguments);

        /**
         * @brief Builds action descriptor for this task.
         * @return Action descriptor - the time correction task.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief Time correction action, that will correct current time based on the external RTC.
         * @param[in] state Reference to global mission state.
         */
        void Save(const SystemState& state);

        bool SaveToFile(gsl::span<const std::uint8_t> frame);

      private:
        /**
         * @brief Condition for time correction action.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         * @return true if correction action should be executed, false otherwise.
         */
        static bool SaveCondition(const SystemState& state, void* param);

        /**
         * @brief Time correction action, that will correct current time based on the external RTC.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static void SaveProxy(const SystemState& state, void* param);

        services::fs::IFileSystem& provider;

        TelemetryConfiguration configuration;
    };

    /** @} */
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TIME_HPP_ */
