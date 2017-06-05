#include "base/BitWriter.hpp"
#include "gyro/telemetry.hpp"

namespace devices
{
    namespace gyro
    {
        GyroscopeTelemetry::GyroscopeTelemetry() : x(0), y(0), z(0), temperature(0)
        {
        }

        GyroscopeTelemetry::GyroscopeTelemetry(int16_t xMes, int16_t yMes, int16_t zMes, int16_t temp)
            : x(xMes), y(yMes), z(zMes), temperature(temp)
        {
        }

        void GyroscopeTelemetry::Write(BitWriter& writer) const
        {
            writer.Write(static_cast<std::uint16_t>(this->x));
            writer.Write(static_cast<std::uint16_t>(this->y));
            writer.Write(static_cast<std::uint16_t>(this->z));
            writer.Write(static_cast<std::uint16_t>(this->temperature));
        }
    }
}
