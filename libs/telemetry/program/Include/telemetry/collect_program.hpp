#ifndef LIBS_TELEMETRY_PROGRAM_CRC_COLLECT_HPP
#define LIBS_TELEMETRY_PROGRAM_CRC_COLLECT_HPP

#pragma once

#include "antenna/antenna.h"
#include "mission/base.hpp"
#include "program_flash/boot_table.hpp"
#include "telemetry/state.hpp"

namespace telemetry
{
    /**
     * @brief This task is responsible for acquiring & updating running program crc value.
     * @telemetry_acquisition
     * @ingroup telemetry
     */
    class ProgramCrcTelemetryAcquisition : public mission::Update
    {
      public:
        /**
         * @brief ctor.
         * @param bootTable Reference to boot table
         */
        ProgramCrcTelemetryAcquisition(program_flash::BootTable& bootTable);

        /**
         * @brief Builds update descriptor for this task.
         * @return Update descriptor - the antenna telemetry acquisition update task.
         */
        mission::UpdateDescriptor<telemetry::TelemetryState> BuildUpdate();

        /**
         * @brief Acquires running program crc value & stores it in passed state object.
         * @param[in] state Object that should be updated with new running program crc value.
         * @return Telemetry acquisition result.
         */
        mission::UpdateResult UpdateTelemetry(telemetry::TelemetryState& state);

      private:
        /**
         * @brief Updates current running program crc value in global state.
         * @param[in] state Reference to global state.
         * @param[in] param Current execution context.
         */
        static mission::UpdateResult UpdateProc(telemetry::TelemetryState& state, void* param);

        std::uint32_t GetLength(std::uint8_t index);

        /** @brief Boot table */
        program_flash::BootTable& _bootTable;
    };
}

#endif
