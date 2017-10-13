#include "telemetry/SystemStartup.hpp"
#include "base/BitWriter.hpp"

namespace telemetry
{
    SystemStartup::SystemStartup() : bootCounter(0), bootReason(0), bootIndex(0)
    {
    }

    SystemStartup::SystemStartup(std::uint32_t counter, std::uint8_t index, std::uint16_t reason) //
        : bootCounter(counter),
          bootReason(reason),
          bootIndex(index)
    {
    }

    void SystemStartup::Write(BitWriter& writer) const
    {
        writer.Write(this->bootCounter);
        writer.Write(this->bootIndex);
        writer.Write(this->bootReason);
    }
}
