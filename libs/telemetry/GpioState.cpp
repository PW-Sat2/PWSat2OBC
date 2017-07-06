#include "base/BitWriter.hpp"
#include "telemetry/gpio.hpp"

namespace telemetry
{
    GpioState::GpioState(bool sailState) : isDeployed(sailState)
    {
    }

    void GpioState::Write(BitWriter& writer) const
    {
        writer.Write(this->isDeployed);
    }
}
