#include "telemetry.h"

using namespace devices::payload;

bool PayloadTelemetry::Photodiodes::Write(Writer& writer)
{
    writer.WriteWordLE(Xp);
    writer.WriteWordLE(Xn);
    writer.WriteWordLE(Yp);
    writer.WriteWordLE(Yn);

    return writer.Status();
}
