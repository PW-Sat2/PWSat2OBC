#include "mission/TelemetrySerialization.hpp"
#include <cassert>
#include <cstring>
#include "base/BitWriter.hpp"
#include "logger/logger.h"

namespace telemetry
{
    using namespace std::chrono_literals;

    TelemetrySerialization::TelemetrySerialization(int /*p*/)
    {
    }

    mission::UpdateDescriptor<TelemetryState> TelemetrySerialization::BuildUpdate()
    {
        mission::UpdateDescriptor<telemetry::TelemetryState> descriptor;
        descriptor.name = "Prepare serialized telemetry";
        descriptor.param = this;
        descriptor.updateProc = Proxy;
        return descriptor;
    }

    mission::UpdateResult TelemetrySerialization::SaveTelemetry(TelemetryState& state)
    {
        decltype(TelemetryState::lastSerializedTelemetry) buffer;
        BitWriter writer(buffer);
        state.telemetry.Write(writer);
        assert(writer.Status());
        if (!writer.Status())
        {
            LOGF(LOG_LEVEL_ERROR, "Insufficient buffer space for telemetry: '%d'.", static_cast<int>(writer.GetBitDataLength()));
            return mission::UpdateResult::Failure;
        }

        auto content = writer.Capture();
        if (content.empty())
        {
            return mission::UpdateResult::Warning;
        }

        Lock lock(state.bufferLock, 5s);
        if (static_cast<bool>(lock))
        {
            std::memcpy(state.lastSerializedTelemetry.data(), buffer.data(), buffer.size());
        }

        return mission::UpdateResult::Ok;
    }

    mission::UpdateResult TelemetrySerialization::Proxy(TelemetryState& state, void* param)
    {
        const auto This = static_cast<TelemetrySerialization*>(param);
        return This->SaveTelemetry(state);
    }
}
