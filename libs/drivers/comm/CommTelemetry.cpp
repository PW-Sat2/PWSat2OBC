#include "CommTelemetry.hpp"
#include "base/BitWriter.hpp"

COMM_BEGIN

CommTelemetry::CommTelemetry()
{
}

CommTelemetry::CommTelemetry(const TransmitterTelemetry& transmitter, const ReceiverTelemetry& receiver)
    : _transmitter(transmitter), _receiver(receiver)
{
}

void CommTelemetry::Write(BitWriter& /*writer*/) const
{
}

COMM_END
