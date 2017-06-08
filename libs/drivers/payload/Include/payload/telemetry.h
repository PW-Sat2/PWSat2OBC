#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_

#include <cstdint>

namespace drivers
{
    namespace payload
    {
        struct PayloadTelemetry
        {
            struct SunsRef
            {
                std::array<std::uint16_t, 5> voltages;
            };

            struct Temperatures
            {
                std::uint16_t supply;
                std::uint16_t Xp;
                std::uint16_t Xn;
                std::uint16_t Yp;
                std::uint16_t Yn;
                std::uint16_t sads;
                std::uint16_t sail;
                std::uint16_t cam_nadir;
                std::uint16_t cam_wing;
            };

            struct Photodiodes
            {
                std::uint16_t Xp;
                std::uint16_t Xn;
                std::uint16_t Yp;
                std::uint16_t Yn;
            };

            struct Housekeeping
            {
                std::uint16_t int_3v3d;
                std::uint16_t obc_3v3d;
            };

            struct Radfet
            {
                std::uint32_t temperature;
                std::array<std::uint32_t, 3> vth;
            };

            SunsRef suns_ref;
            Temperatures temperatures;
            Photodiodes photodiodes;
            Housekeeping housekeeping;
            Radfet radfet;
        };

        static_assert(sizeof(PayloadTelemetry) == 56, "Incorrect size of Telemetry structure (padding?)");
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_ */
