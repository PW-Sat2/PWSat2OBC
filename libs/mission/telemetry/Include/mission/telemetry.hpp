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
     * @brief Module that saves current telemetry state to files.
     * @{
     */

    /**
     * @brief This type contains configuration of telemetry saving task.
     */
    struct TelemetryConfiguration
    {
        /**
         * @brief Path to current telemetry event file.
         */
        const char* currentFileName;

        /**
         * @brief Path to previous telemetry event file.
         */
        const char* previousFileName;

        /**
         * @brief Maximal size of single telemetry event file.
         */
        std::int32_t maxFileSize;
    };

    /**
     * @brief This task is responsible for observing the telemetry container state and as soon
     * as change is observed extract save it to telemetry event file.
     *
     * This task uses two files for saving the state of the telemetry changes:
     * - current telemetry file - This file is actively used and contains the most recent telemetry changes.
     * - previous telemetry file - This is archival file that contains telemetry from the
     * longer period of time that immediately precedes the time period covered by the
     * \a current \a telemetry \a file
     *
     * The newest changes to the telemetry elements are being added to the end of the
     * \a current \a telemetry \a file. Once the current telemetry file reaches configured size
     * it is archived.
     *
     * The telemetry archivization process is done by removing \a previous \a telemetry \a file and
     * changing \a current \a telemetry \a file to namd of the \a previous \a telemetry \a file.
     */
    class TelemetryTask : public Action
    {
      public:
        /**
         * @brief ctor.
         * @param[in] arguments Reference to file system provider & current task configuration.
         */
        TelemetryTask(std::tuple<services::fs::IFileSystem&, TelemetryConfiguration> arguments);

        /**
         * @brief Builds action descriptor for this task.
         * @return Action descriptor - the telemetry change save task.
         */
        ActionDescriptor<SystemState> BuildAction();

        /**
         * @brief This procedure extracts modified parts of the telemetry and saves it to the telemetry
         * event file.
         *
         * Once the process is complete the telemetry container is notified that all changes are saved.
         * @param[in] state Reference to global mission state.
         */
        void Save(const SystemState& state);

        /**
         * @brief This procedure is responsible for appending the passed data frame to the current
         * telemetry event file.
         *
         * @param[in] buffer Buffer with data frame that should be added to file.
         * @return Operation status, true on success, false otherwise.
         */
        bool SaveToFile(gsl::span<const std::uint8_t> buffer);

      private:
        /**
         * @brief Condition for telemetry saving action.
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         * @return true if there are telemetry changes that should be saved, false otherwise.
         */
        static bool SaveCondition(const SystemState& state, void* param);

        /**
         * @brief This procedure is responsible for appending the passed data frame to the current
         * telemetry event file.
         *
         * @param[in] state Reference to global mission state.
         * @param[in] param Current execution context.
         */
        static void SaveProxy(const SystemState& state, void* param);

        /**
         * @brief File system provider.
         */
        services::fs::IFileSystem& provider;

        /**
         * @bier Current configuration
         */
        TelemetryConfiguration configuration;
    };

    /** @} */
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_TELEMETRY_HPP_ */
