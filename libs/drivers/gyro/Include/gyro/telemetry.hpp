#ifndef LIBS_DRIVERS_GYRO_TELEMETRY_HPP
#define LIBS_DRIVERS_GYRO_TELEMETRY_HPP

#pragma once

#include <cstdint>
#include "base/fwd.hpp"

namespace devices
{
    namespace gyro
    {
        /**
         * @addtogroup ITG-3200
         * @{
         */

        /**
         * @brief Data read from gyroscope.
         * @telemetry_element
         */
        class GyroscopeTelemetry
        {
          public:
            /**
             * @brief GyroscopeTelemetry telemetry unique identifier.
             */
            static constexpr std::uint32_t Id = 4;

            /**
             * @brief ctor.
             */
            GyroscopeTelemetry();

            /**
             * @brief ctor.
             * @param[in] xMes Gyroscope measurement along x axis.
             * @param[in] yMes Gyroscope measurement along y axis.
             * @param[in] zMes Gyroscope measurement along z axis.
             * @param[in] temp Gyroscope temperature.
             */
            GyroscopeTelemetry(int16_t xMes, int16_t yMes, int16_t zMes, int16_t temp);

            /**
             * @brief Returns gyroscope measurement along x axis.
             * @return Gyroscope measurement along x axis.
             */
            int16_t X() const noexcept;

            /**
             * @brief Returns gyroscope measurement along y axis.
             * @return Gyroscope measurement along y axis.
             */
            int16_t Y() const noexcept;

            /**
             * @brief Returns gyroscope measurement along z axis.
             * @return Gyroscope measurement along z axis.
             */
            int16_t Z() const noexcept;

            /**
             * @brief Returns gyroscope temperature.
             * @return Gyroscope temperature.
             */
            int16_t Temperature() const noexcept;

            /**
             * @brief Read the system startup telemetry element subsystem state from passed reader.
             * @param[in] reader Buffer reader that should be used to read the serialized state.
             */
            void Read(Reader& reader);

            /**
             * @brief Write the system startup telemetry element to passed buffer writer object.
             * @param[in] writer Buffer writer object that should be used to write the serialized state.
             */
            void Write(Writer& writer) const;

            /**
             * @brief Returns size of the serialized state in bytes.
             * @return Size of the serialized state in bytes.
             */
            static constexpr std::uint32_t Size();

            /**
             * @brief Reports when two system startup objects are different.
             *
             * @remark Used by Telemetry container.
             * @param[in] state Object to compare to.
             * @return True whether the two objects are different, false otherwise.
             */
            bool IsDifferent(const GyroscopeTelemetry& state) const;

          private:
            int16_t x;           ///< Measurement along x axis.
            int16_t y;           ///< Measurement along y axis.
            int16_t z;           ///< Measurement along z axis.
            int16_t temperature; ///< Gyroscope temperature.
        };

        inline int16_t GyroscopeTelemetry::X() const noexcept
        {
            return this->x;
        }

        inline int16_t GyroscopeTelemetry::Y() const noexcept
        {
            return this->y;
        }

        inline int16_t GyroscopeTelemetry::Z() const noexcept
        {
            return this->z;
        }

        inline int16_t GyroscopeTelemetry::Temperature() const noexcept
        {
            return this->temperature;
        }

        constexpr std::uint32_t GyroscopeTelemetry::Size()
        {
            return 4 * sizeof(int16_t);
        }

        inline bool GyroscopeTelemetry::IsDifferent(const GyroscopeTelemetry& state) const
        {
            return this->x != state.x || //
                this->y != state.y ||    //
                this->z != state.z ||    //
                this->temperature != state.temperature;
        }

        /** @} */
    }
}

#endif
