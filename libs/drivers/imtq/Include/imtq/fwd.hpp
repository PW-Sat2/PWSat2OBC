#ifndef LIBS_DRIVERS_IMTQ_INCLUDE_IMTQ_FWD_HPP_
#define LIBS_DRIVERS_IMTQ_INCLUDE_IMTQ_FWD_HPP_

#pragma once

#include <cstdint>

namespace devices
{
    namespace imtq
    {
        class ImtqDriver;

        /**
         * @brief Current as returned by imtq. 1 LSB = 1e-4 A
         */
        using Current = std::int16_t;

        /**
         * @brief Magnetic dipole as returned by imtq. 1 LSB = 1e-4 Am^2
         */
        using Dipole = std::int16_t;

        /**
         * @brief Magnetic field measurement as returned by imtq. 1 LSB = 1e-7 T
         */
        using MagnetometerMeasurement = std::int32_t;

        /**
         * @brief BDot algorithm value: representation - 1e-9 T/s
         */
        using BDotType = std::int32_t;

        /**
         * @brief Imtq internal mode
         */
        enum class Mode : std::uint8_t
        {
            /**
             * @brief MTM measurements and MTQ actuation are available upon request.
             * The user controls the subsystem via I2C commands.
             */
            Idle = 0,

            /**
             * @brief Actuate the MTQ in sequence and take measurements from all the sensors.
             */
            Selftest = 1,

            /**
             * @brief The BDOT algorithm is performed based on the MTM measurements and actuates the MTQ.
             * The state of the controller is available on request at any time.
             */
            Detumble = 2
        };
    }
}

#endif
