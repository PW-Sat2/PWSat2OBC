#include "telemetry.h"
#include "base/Writer.h"

namespace devices
{
    namespace payload
    {
        void PayloadTelemetry::SunsRef::Write(Writer& writer)
        {
            for (auto v : this->voltages)
            {
                writer.WriteWordLE(v);
            }
        }

        void PayloadTelemetry::Temperatures::Write(Writer& writer)
        {
            writer.WriteWordLE(this->supply);
            writer.WriteWordLE(this->Xp);
            writer.WriteWordLE(this->Xn);
            writer.WriteWordLE(this->Yp);
            writer.WriteWordLE(this->Yn);
            writer.WriteWordLE(this->sads);
            writer.WriteWordLE(this->sail);
            writer.WriteWordLE(this->cam_nadir);
            writer.WriteWordLE(this->cam_wing);
        }

        bool PayloadTelemetry::Photodiodes::Write(Writer& writer)
        {
            writer.WriteWordLE(this->Xp);
            writer.WriteWordLE(this->Xn);
            writer.WriteWordLE(this->Yp);
            writer.WriteWordLE(this->Yn);

            return writer.Status();
        }
    }
}
