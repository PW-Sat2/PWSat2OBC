#include "telemetry/ErrorCounters.hpp"
#include <algorithm>
#include "base/BitWriter.hpp"

namespace telemetry
{
    ErrorCountingTelemetry::ErrorCountingTelemetry()
    {
        std::fill(this->array.begin(), this->array.end(), 0);
    }

    ErrorCountingTelemetry::ErrorCountingTelemetry(const Container& state)
    {
        std::copy(state.begin(), state.end(), this->array.begin());
    }

    void ErrorCountingTelemetry::Write(BitWriter& writer) const
    {
        for (auto value : this->array)
        {
            writer.Write(value);
        }
    }
}
