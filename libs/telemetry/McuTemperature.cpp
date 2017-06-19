#include "telemetry/McuTemperature.hpp"
#include "base/BitWriter.hpp"

namespace telemetry
{
    McuTemperature::McuTemperature(TemperatureType value) : temperature(value)
    {
    }

    void McuTemperature::Write(BitWriter& writer) const
    {
        writer.Write(this->temperature);
    }
}
