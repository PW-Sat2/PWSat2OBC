#ifndef LIBS_TELEMETRY_IMTQ_HPP
#define LIBS_TELEMETRY_IMTQ_HPP

#pragma once

#include <array>
#include "base/fwd.hpp"
#include "imtq/fwd.hpp"
#include "utils.h"

namespace telemetry
{
    class ImtqTelemetry
    {
      public:
        ImtqTelemetry();

        ImtqTelemetry(std::array<devices::imtq::MagnetometerMeasurement, 3> magnetometerMeasurenemts,
            std::array<devices::imtq::Dipole, 3> measuredDipoles,
            bool areCoilsActive);

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

        const std::array<devices::imtq::MagnetometerMeasurement, 3>& MagnetometerMeasurements() const noexcept;

        const std::array<devices::imtq::Dipole, 3>& Dipoles() const noexcept;

        bool AreCoilsActive() const noexcept;

      private:
        std::array<devices::imtq::MagnetometerMeasurement, 3> magnetometers;
        std::array<devices::imtq::Dipole, 3> dipoles;
        bool coilsActive;
    };

    inline constexpr std::uint32_t ImtqTelemetry::BitSize()
    {
        return Aggregate<decltype(magnetometers), decltype(dipoles), decltype(coilsActive)>;
    }

    inline const std::array<devices::imtq::MagnetometerMeasurement, 3>& ImtqTelemetry::MagnetometerMeasurements() const noexcept
    {
        return this->magnetometers;
    }

    inline const std::array<devices::imtq::Dipole, 3>& ImtqTelemetry::Dipoles() const noexcept
    {
        return this->dipoles;
    }

    inline bool ImtqTelemetry::AreCoilsActive() const noexcept
    {
        return this->coilsActive;
    }

    template <size_t size> struct TD;

    static_assert(ImtqTelemetry::BitSize() == 145, "Invalid serialized size");

    class ImtqBDotTelemetry
    {
      public:
        ImtqBDotTelemetry();

        ImtqBDotTelemetry(const std::array<devices::imtq::BDotType, 3>& bdotValues);

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

        const std::array<devices::imtq::BDotType, 3> BDotValues() const;

      private:
        std::array<devices::imtq::BDotType, 3> bdot;
    };

    inline constexpr std::uint32_t ImtqBDotTelemetry::BitSize()
    {
        return BitLength<decltype(bdot)>;
    }

    inline const std::array<devices::imtq::BDotType, 3> ImtqBDotTelemetry::BDotValues() const
    {
        return this->bdot;
    }

    static_assert(ImtqBDotTelemetry::BitSize() == 96, "Invalid serialized size");

    class ImtqHousekeeping
    {
      public:
        ImtqHousekeeping();

        ImtqHousekeeping(std::uint16_t digitalVoltage,
            std::uint16_t analogVoltage,
            std::uint16_t digitalCurrent,
            std::uint16_t analogCurrent,
            std::array<std::uint16_t, 3> coilCurrents,
            std::array<std::uint16_t, 3> coilTemperatures,
            std::uint16_t mcuTemperature);

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

      private:
        std::uint16_t digitalVoltage;
        std::uint16_t analogVoltage;
        std::uint16_t digitalCurrent;
        std::uint16_t analogCurrent;
        std::array<std::uint16_t, 3> coilCurrents;
        std::array<std::uint16_t, 3> coilTemperatures;
        std::uint16_t mcuTemperature;
    };

    constexpr std::uint32_t ImtqHousekeeping::BitSize()
    {
        return Aggregate<decltype(digitalVoltage),
            decltype(analogVoltage),
            decltype(digitalCurrent),
            decltype(analogCurrent),
            decltype(coilCurrents),
            decltype(coilTemperatures),
            decltype(mcuTemperature)>;
    }

    static_assert(ImtqHousekeeping::BitSize() == 176, "Invalid serialized size");

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

    class ImtqSelfTest
    {
      public:
        ImtqSelfTest();

        ImtqSelfTest(const std::array<std::uint8_t, 8>& result);

        void Write(BitWriter& writer) const;

        static constexpr std::uint32_t BitSize();

        const std::array<std::uint8_t, 8>& Result() const;

      private:
        std::array<std::uint8_t, 8> seflTestResult;
    };

    constexpr std::uint32_t ImtqSelfTest::BitSize()
    {
        return BitLength<decltype(seflTestResult)>;
    }

    inline const std::array<std::uint8_t, 8>& ImtqSelfTest::Result() const
    {
        return this->seflTestResult;
    }

    static_assert(ImtqSelfTest::BitSize() == 64, "Invalid serialized size");
}

#endif
