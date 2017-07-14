#ifndef LIBS_TELEMETRY_STATE_HPP
#define LIBS_TELEMETRY_STATE_HPP

#pragma once

#include "BasicTelemetry.hpp"
#include "ErrorCounters.hpp"
#include "Experiments.hpp"
#include "SystemStartup.hpp"
#include "Telemetry.hpp"
#include "TimeTelemetry.hpp"
#include "antenna/telemetry.hpp"
#include "comm/CommTelemetry.hpp"
#include "fwd.hpp"
#include "gyro/telemetry.hpp"
#include "state/time/TimeState.hpp"
#include "system.h"

namespace telemetry
{
    /**
     * @brief This type represents state of telemetry acquisition loop.
     * @ingroup telemetry
     */
    struct TelemetryState
    {
        /**
         * @brief Initializes telemetry state.
         * @return Operation status, true in case of success, false otherwise.
         */
        bool Initialize();

        /**
         * @brief Container for all telemetry elements currently collected by acquisition loop.
         */
        ManagedTelemetry telemetry;

        /**
         * @brief Handle to semaphore that protects access to serialized telemetry.
         */
        OSSemaphoreHandle bufferLock;

        /**
         * @brief Buffer that contains serialized state of the last seen telemetry state.
         */
        std::array<std::uint8_t, ManagedTelemetry::TotalSerializedSize> lastSerializedTelemetry;
    };

    static_assert(ProgramState::BitSize() == 16, "Invalid serialized size");
    static_assert(McuTemperature::BitSize() == 12, "Invalid serialized size");
    static_assert(FileSystemTelemetry::BitSize() == 32, "Invalid serialized size");
    static_assert(GpioState::BitSize() == 1, "Invalid serialized size");

    static_assert(ManagedTelemetry::TotalSerializedSize <= (devices::comm::MaxDownlinkFrameSize - 1), "Telemetry is too large");
}

#endif
