#include "telemetry/TimeTelemetry.hpp"
#include "base/BitWriter.hpp"

namespace telemetry
{
    using namespace std::chrono_literals;

    InternalTimeTelemetry::InternalTimeTelemetry() : time(0ms)
    {
    }

    InternalTimeTelemetry::InternalTimeTelemetry(std::chrono::milliseconds newTime) : time(newTime)
    {
    }

    void InternalTimeTelemetry::Write(BitWriter& writer) const
    {
        writer.Write(static_cast<std::uint64_t>(this->time.count()));
    }

    ExternalTimeTelemetry::ExternalTimeTelemetry() : time(0s)
    {
    }

    ExternalTimeTelemetry::ExternalTimeTelemetry(std::chrono::seconds newTime) : time(newTime)
    {
    }

    void ExternalTimeTelemetry::Write(BitWriter& writer) const
    {
        writer.Write(static_cast<std::uint32_t>(this->time.count()));
    }
}
