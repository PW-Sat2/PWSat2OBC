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
             * @brief Base class for payload telemetry types
             */
            template <const uint8_t TDeviceDataAddress, const uint8_t TDeviceDataLength> struct PayloadTelemetryBase
            {
                /** @brief Address from which data starts */
                static constexpr uint8_t DeviceDataAddress = TDeviceDataAddress;
                /** @brief Length of data in bytes */
                static constexpr uint8_t DeviceDataLength = TDeviceDataLength;
            };

            /**
             * @brief Status registers.
             *
             */
            struct Status : PayloadTelemetryBase<0, 1>
            {
                /** @brief Who Am I */
                uint8_t who_am_i;
            };

            /**
             * @brief SunS Reference Voltages telemetry structure.
             */
            struct SunsRef : PayloadTelemetryBase<1, 10>
            {
                /** @brief SunS voltages */
                std::array<std::uint16_t, 5> voltages;
            };

            /**
             * @brief Temperetures telemetry structure.
             */
            struct Temperatures : PayloadTelemetryBase<11, 18>
            {
                /** @brief Supply temperature*/
                std::uint16_t supply;
                /** @brief X+ temperature*/
                std::uint16_t Xp;
                /** @brief X- temperature*/
                std::uint16_t Xn;
                /** @brief Y+ temperature*/
                std::uint16_t Yp;
                /** @brief Y- temperature*/
                std::uint16_t Yn;
                /** @brief SADS temperature*/
                std::uint16_t sads;
                /** @brief SAIL temperature*/
                std::uint16_t sail;
                /** @brief CAMnadir temperature*/
                std::uint16_t cam_nadir;
                /** @brief CAMwing temperature*/
                std::uint16_t cam_wing;
            };

            /**
             * @brief Photodiodes telemetry structure.
             */
            struct Photodiodes : PayloadTelemetryBase<29, 8>
            {
                /** @brief X+ photodiode measurement */
                std::uint16_t Xp;
                /** @brief X- photodiode measurement */
                std::uint16_t Xn;
                /** @brief Y+ photodiode measurement */
                std::uint16_t Yp;
                /** @brief Y- photodiode measurement */
                std::uint16_t Yn;
            };

            /**
             * @brief Housekeeping data telemetry structure.
             */
            struct Housekeeping : PayloadTelemetryBase<37, 4>
            {
                /** @brief INT 3V3 */
                std::uint16_t int_3v3d;
                /** @brief OBC 3V3 */
                std::uint16_t obc_3v3d;
            };

            /**
             * @brief RadFET telemetry structure.
             */
            struct Radfet : PayloadTelemetryBase<41, 17>
            {
                /** @brief Status */
                std::uint8_t status;
                /** @brief Temperature */
                std::uint32_t temperature;
                /** @brief VTH */
                std::array<std::uint32_t, 3> vth;
            };
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_ */
