#include "telemetry.h"
#include "base/Writer.h"

namespace devices
{
    namespace payload
    {
        bool PayloadTelemetry::SunsRef::Write(Writer& writer) const
        {
            for (auto v : this->voltages)
            {
                writer.WriteWordLE(v);
            }

            return writer.Status();
        }

        bool PayloadTelemetry::Temperatures::Write(Writer& writer) const
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
            return writer.Status();
        }

        bool PayloadTelemetry::Photodiodes::Write(Writer& writer) const
        {
            writer.WriteWordLE(this->Xp);
            writer.WriteWordLE(this->Xn);
            writer.WriteWordLE(this->Yp);
            writer.WriteWordLE(this->Yn);

            return writer.Status();
        }

        bool PayloadTelemetry::Status::Write(Writer& writer) const
        {
            writer.WriteByte(this->who_am_i);
            return writer.Status();
        }

        bool PayloadTelemetry::Housekeeping::Write(Writer& writer) const
        {
            writer.WriteWordLE(this->int_3v3d);
            writer.WriteWordLE(this->obc_3v3d);
            return writer.Status();
        }

        bool PayloadTelemetry::Radfet::Write(Writer& writer) const
        {
            writer.WriteByte(this->status);
            writer.WriteDoubleWordLE(this->temperature);
            for (auto v : this->vth)
            {
                writer.WriteDoubleWordLE(v);
            }

            return writer.Status();
        }
    }
}
