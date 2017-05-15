#include "telemetry/SystemStartup.hpp"
#include "base/reader.h"
#include "base/writer.h"

namespace telemetry
{
    SystemStartup::SystemStartup() : bootCounter(0), bootIndex(0)
    {
    }

    SystemStartup::SystemStartup(std::uint32_t counter, std::uint8_t index) : bootCounter(counter), bootIndex(index)
    {
    }

    void SystemStartup::Read(Reader& reader)
    {
        this->bootCounter = reader.ReadQuadWordLE();
        this->bootIndex = reader.ReadByte();
    }

    void SystemStartup::Write(Writer& writer) const
    {
        writer.WriteQuadWordLE(this->bootCounter);
        writer.WriteByte(this->bootIndex);
    }
}
