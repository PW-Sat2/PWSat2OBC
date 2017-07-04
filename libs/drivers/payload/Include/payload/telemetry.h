#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_

#include <cstdint>

namespace drivers
{
    namespace payload
    {
        /**
         * @defgroup payload_telemetry Payload Telemetry definition
         * @ingroup payload
         *
         * @brief This module contains definition of Payload Telemetry data.
         *
         * This module contains definition of Payload Telemetry data.
         * It is used for calculatin addresses of required data and conversion from I2C buffers to meaningful data.
         *
         * @{
         */

        /**
         * @brief Payload Telemetry definition
         */
        struct PayloadTelemetry
        {
            template <const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength> struct PayloadTelemetryBase
            {
                static constexpr uint8_t DeviceDataAddress = TDeviceDataAddress;
                static constexpr uint8_t DeviceDataLength = TDeviceDataLength;
            };

            /**
             * @brief Status registers.
             *
             */
            struct Status : PayloadTelemetryBase<0, 1>
            {
                uint8_t who_am_i;
            };

            /**
             * @brief SunS Reference Voltages telemetry structure.
             */
            struct SunsRef : PayloadTelemetryBase<1, 10>
            {
                std::array<std::uint16_t, 5> voltages;
            };

            /**
             * @brief Temperetures telemetry structure.
             */
            struct Temperatures : PayloadTelemetryBase<11, 18>
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

            /**
             * @brief Photodiodes telemetry structure.
             */
            struct Photodiodes : PayloadTelemetryBase<29, 8>
            {
                std::uint16_t Xp;
                std::uint16_t Xn;
                std::uint16_t Yp;
                std::uint16_t Yn;
            };

            /**
             * @brief Housekeeping data telemetry structure.
             */
            struct Housekeeping : PayloadTelemetryBase<37, 4>
            {
                std::uint16_t int_3v3d;
                std::uint16_t obc_3v3d;
            };

            /**
             * @brief RadFET telemetry structure.
             */
            struct Radfet : PayloadTelemetryBase<41, 17>
            {
                std::uint8_t status;
                std::uint32_t temperature;
                std::array<std::uint32_t, 3> vth;
            };
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_ */
