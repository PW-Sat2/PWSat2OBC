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
            /**
             * @brief Status registers.
             *
             */
            struct Status
            {
                uint8_t who_am_i;
                static constexpr uint8_t DeviceDataAddress = 0;
                static constexpr uint8_t DeviceDataLength = 1;
            };

            /**
             * @brief SunS Reference Voltages telemetry structure.
             */
            struct SunsRef
            {
                std::array<std::uint16_t, 5> voltages;
                static constexpr uint8_t DeviceDataAddress = 1;
                static constexpr uint8_t DeviceDataLength = 10;
            };

            /**
             * @brief Temperetures telemetry structure.
             */
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
                static constexpr uint8_t DeviceDataAddress = 11;
                static constexpr uint8_t DeviceDataLength = 18;
            };

            /**
             * @brief Photodiodes telemetry structure.
             */
            struct Photodiodes
            {
                std::uint16_t Xp;
                std::uint16_t Xn;
                std::uint16_t Yp;
                std::uint16_t Yn;
                static constexpr uint8_t DeviceDataAddress = 29;
                static constexpr uint8_t DeviceDataLength = 8;
            };

            /**
             * @brief Housekeeping data telemetry structure.
             */
            struct Housekeeping
            {
                std::uint16_t int_3v3d;
                std::uint16_t obc_3v3d;
                static constexpr uint8_t DeviceDataAddress = 37;
                static constexpr uint8_t DeviceDataLength = 4;
            };

            /**
             * @brief RadFET telemetry structure.
             */
            struct Radfet
            {
                std::uint8_t status;
                std::uint32_t temperature;
                std::array<std::uint32_t, 3> vth;
                static constexpr uint8_t DeviceDataAddress = 41;
                static constexpr uint8_t DeviceDataLength = 17;
            };
        };

        static_assert(std::is_pod<PayloadTelemetry>::value, "POD");

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_ */
