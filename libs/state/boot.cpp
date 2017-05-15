#include "base/reader.h"
#include "base/writer.h"
#include "boot/BootState.hpp"

namespace state
{
    BootState::BootState() : bootCounter(0)
    {
    }

    BootState::BootState(std::uint32_t value) : bootCounter(value)
    {
    }

    void BootState::Read(Reader& reader)
    {
        this->bootCounter = reader.ReadDoubleWordLE();
    }

    void BootState::Write(Writer& writer) const
    {
        writer.WriteDoubleWordLE(this->bootCounter);
    }
}
