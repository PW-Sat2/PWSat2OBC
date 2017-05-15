#include "base/reader.h"
#include "base/writer.h"
#include "gyro/telemetry.hpp"

namespace devices
{
    namespace gyro
    {
        GyroscopeTelemetry::GyroscopeTelemetry() : x(0), y(0), z(0), temperature(0)
        {
        }

        GyroscopeTelemetry::GyroscopeTelemetry(int16_t xMes, int16_t yMes, int16_t zMEs, int16_t temp)
            : x(xMes), y(yMes), z(zMEs), temperature(temp)
        {
        }

        void GyroscopeTelemetry::Read(Reader& reader)
        {
            this->x = reader.ReadSignedWordLE();
            this->y = reader.ReadSignedWordLE();
            this->z = reader.ReadSignedWordLE();
            this->temperature = reader.ReadSignedWordLE();
        }

        void GyroscopeTelemetry::Write(Writer& writer) const
        {
            writer.WriteSignedWordLE(this->x);
            writer.WriteSignedWordLE(this->y);
            writer.WriteSignedWordLE(this->z);
            writer.WriteSignedWordLE(this->temperature);
        }
    }
}
