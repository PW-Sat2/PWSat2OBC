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
            };

            /**
             * @brief SunS Reference Voltages telemetry structure.
             */
            struct SunsRef
            {
                std::array<std::uint16_t, 5> voltages;
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
            };

            /**
             * @brief Housekeeping data telemetry structure.
             */
            struct Housekeeping
            {
                std::uint16_t int_3v3d;
                std::uint16_t obc_3v3d;
            };

            /**
             * @brief RadFET telemetry structure.
             */
            struct Radfet
            {
                std::uint32_t temperature;
                std::array<std::uint32_t, 3> vth;
            };

            /**
             * @brief Who Am I register data.
             */
            Status status;

            /**
             * @brief RadFET telemetry data.
             */
            SunsRef suns_ref;

            /**
             * @brief Temperature telemetry data.
             */
            Temperatures temperatures;

            /**
             * @brief Photodiodes telemetry data.
             */
            Photodiodes photodiodes;

            /**
             * @brief Housekeeping telemetry data.
             */
            Housekeeping housekeeping;

            /**
             * @brief RadFET telemetry data.
             */
            Radfet radfet;
        }
        /**
         * @brief Packing of structure to ensure contignous address space equal to Payload address space.
         */
        __attribute__((packed));

        static_assert(sizeof(PayloadTelemetry) == 57, "Incorrect size of Telemetry structure (padding?)");
        static_assert(std::is_pod<PayloadTelemetry>::value, "POD");

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_TELEMETRY_H_ */
