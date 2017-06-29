#include "beacon.hpp"
#include <chrono>
#include "base/writer.h"
#include "downlink.h"
#include "logger/logger.h"
#include "telemetry/state.hpp"

using namespace std::chrono_literals;

bool WriteBeaconPayload(telemetry::TelemetryState& state, Writer& writer)
{
    {
        Lock lock(state.bufferLock, 5s);
        if (!static_cast<bool>(lock))
        {
            LOG(LOG_LEVEL_ERROR, "[beacon] Unable to acquire access to telemetry.");
            return false;
        }

        writer.Reset();
        writer.WriteByte(telecommunication::downlink::BeaconMarker);
        writer.WriteArray(state.lastSerializedTelemetry);
    }

    if (!writer.Status())
    {
        LOG(LOG_LEVEL_ERROR, "[beacon] Unable to fit telemetry in single comm frame.");
    }

    return writer.Status();
}
