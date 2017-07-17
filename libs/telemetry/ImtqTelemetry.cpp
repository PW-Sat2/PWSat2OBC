#include "telemetry/ImtqTelemetry.hpp"
#include <algorithm>
#include "base/BitWriter.hpp"

namespace telemetry
{
    using namespace std::chrono_literals;

    ImtqTelemetry::ImtqTelemetry() : coilsActive(false)
    {
        std::uninitialized_fill(magnetometers.begin(), magnetometers.end(), 0);
        std::uninitialized_fill(dipoles.begin(), dipoles.end(), 0);
    }

    ImtqTelemetry::ImtqTelemetry(std::array<devices::imtq::MagnetometerMeasurement, 3> magnetometerMeasurenemts,
        std::array<devices::imtq::Dipole, 3> measuredDipoles,
        bool areCoilsActive)
        : magnetometers(magnetometerMeasurenemts), //
          dipoles(measuredDipoles),                //
          coilsActive(areCoilsActive)
    {
    }

    void ImtqTelemetry::Write(BitWriter& writer) const
    {
        for (auto value : this->magnetometers)
        {
            writer.Write(gsl::narrow_cast<std::uint32_t>(value));
        }

        for (auto value : this->dipoles)
        {
            writer.Write(gsl::narrow_cast<std::uint16_t>(value));
        }

        writer.Write(this->coilsActive);
    }

    ImtqBDotTelemetry::ImtqBDotTelemetry()
    {
        std::uninitialized_fill(bdot.begin(), bdot.end(), 0);
    }

    ImtqBDotTelemetry::ImtqBDotTelemetry(const std::array<devices::imtq::BDotType, 3>& bdotValues) : bdot(bdotValues)
    {
    }

    void ImtqBDotTelemetry::Write(BitWriter& writer) const
    {
        for (auto value : this->bdot)
        {
            writer.Write(gsl::narrow_cast<std::uint32_t>(value));
        }
    }

    ImtqHousekeeping::ImtqHousekeeping() : digitalVoltage(0), analogVoltage(0), digitalCurrent(0), analogCurrent(0), mcuTemperature(0)
    {
        std::uninitialized_fill(coilCurrents.begin(), coilCurrents.end(), 0);
        std::uninitialized_fill(coilTemperatures.begin(), coilTemperatures.end(), 0);
    }

    ImtqHousekeeping::ImtqHousekeeping(std::uint16_t digitalVoltage_,
        std::uint16_t analogVoltage_,
        std::uint16_t digitalCurrent_,
        std::uint16_t analogCurrent_,
        std::array<std::uint16_t, 3> coilCurrents_,
        std::array<std::uint16_t, 3> coilTemperatures_,
        std::uint16_t mcuTemperature_)
        : digitalVoltage(digitalVoltage_),     //
          analogVoltage(analogVoltage_),       //
          digitalCurrent(digitalCurrent_),     //
          analogCurrent(analogCurrent_),       //
          coilCurrents(coilCurrents_),         //
          coilTemperatures(coilTemperatures_), //
          mcuTemperature(mcuTemperature_)
    {
    }

    void ImtqHousekeeping::Write(BitWriter& writer) const
    {
        writer.Write(this->digitalVoltage);
        writer.Write(this->analogVoltage);
        writer.Write(this->digitalCurrent);
        writer.Write(this->analogCurrent);

        for (auto value : this->coilCurrents)
        {
            writer.Write(value);
        }

        for (auto value : this->coilTemperatures)
        {
            writer.Write(value);
        }

        writer.Write(this->mcuTemperature);
    }

    ImtqState::ImtqState() : status(0), mode(devices::imtq::Mode::Idle), errorCode(0), configurationUpdated(false), uptime(0s)
    {
    }

    ImtqState::ImtqState(std::uint8_t status_, //
        devices::imtq::Mode mode_,
        std::uint8_t errorCode_,
        bool configurationUpdated_,
        std::chrono::seconds uptime_)
        : status(status_),                             //
          mode(mode_),                                 //
          errorCode(errorCode_),                       //
          configurationUpdated(configurationUpdated_), //
          uptime(uptime_)
    {
    }

    void ImtqState::Write(BitWriter& writer) const
    {
        writer.Write(this->status);
        writer.WriteWord(num(this->mode), 2);
        writer.Write(this->errorCode);
        writer.Write(this->configurationUpdated);
        writer.Write(gsl::narrow_cast<std::uint32_t>(this->uptime.count()));
    }

    ImtqSelfTest::ImtqSelfTest()
    {
        std::uninitialized_fill(seflTestResult.begin(), seflTestResult.end(), 0);
    }

    ImtqSelfTest::ImtqSelfTest(const std::array<std::uint8_t, 8>& result) : seflTestResult(result)
    {
    }

    void ImtqSelfTest::Write(BitWriter& writer) const
    {
        for (auto value : this->seflTestResult)
        {
            writer.Write(value);
        }
    }
}
