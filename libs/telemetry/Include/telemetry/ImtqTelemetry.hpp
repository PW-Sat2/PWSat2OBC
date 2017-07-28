#ifndef LIBS_TELEMETRY_IMTQ_HPP
#define LIBS_TELEMETRY_IMTQ_HPP

#pragma once

#include <array>
#include "base/fwd.hpp"
#include "fwd.hpp"
#include "imtq/fwd.hpp"
#include "telemetry/BasicTelemetry.hpp"
#include "utils.h"

namespace telemetry
{
    class ImtqHousekeeping
    {
      public:
        ImtqHousekeeping();

        ImtqHousekeeping(devices::imtq::VoltageInMiliVolt digitalVoltage,
            devices::imtq::VoltageInMiliVolt analogVoltage,
            devices::imtq::Current digitalCurrent,
            devices::imtq::Current analogCurrent,
            devices::imtq::TemperatureMeasurement mcuTemperature);

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

      private:
        devices::imtq::VoltageInMiliVolt digitalVoltage;
        devices::imtq::VoltageInMiliVolt analogVoltage;
        devices::imtq::Current digitalCurrent;
        devices::imtq::Current analogCurrent;
        devices::imtq::TemperatureMeasurement mcuTemperature;
    };

    constexpr std::uint32_t ImtqHousekeeping::BitSize()
    {
        return Aggregate<decltype(digitalVoltage),
            decltype(analogVoltage),
            decltype(digitalCurrent),
            decltype(analogCurrent),
            decltype(mcuTemperature)>;
    }

    static_assert(ImtqHousekeeping::BitSize() == 80, "Invalid serialized size");

    class ImtqState
    {
      public:
        ImtqState();

        ImtqState(std::uint8_t status, //
            devices::imtq::Mode mode,
            std::uint8_t errorCode,
            bool configurationUpdated,
            std::chrono::seconds uptime);

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

      private:
        std::uint8_t status;
        devices::imtq::Mode mode;
        std::uint8_t errorCode;
        bool configurationUpdated;
        std::chrono::seconds uptime;
    };

    constexpr std::uint32_t ImtqState::BitSize()
    {
        return BitLength<decltype(status)> +            //
            2 +                                         // BitLength<decltype(mode)>,
            BitLength<decltype(errorCode)> +            //
            BitLength<decltype(configurationUpdated)> + //
            BitLength<std::uint32_t>;
    }

    static_assert(ImtqState::BitSize() == 51, "Invalid serialized size");
}

#endif
