#include "telemetry/ProgramState.hpp"
#include "base/BitWriter.hpp"

namespace telemetry
{
    void ProgramState::Write(BitWriter& writer) const
    {
        writer.Write(this->programCrc);
    }
}
