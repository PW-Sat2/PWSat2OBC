#ifndef INTERFACE_TYPES_HPP_
#define INTERFACE_TYPES_HPP_

#include <array>
#include <cstdint>

namespace adcs
{
    /**
     * @defgroup adcs_types  ADCS interface types
     *
     * @{
     */

    /** @brief (Draft) Dipole representation
     *          1 LSB = 1e-4 Am^2
     */
    using Dipole = std::int16_t;

    /** @brief Dipole Vector (3d) representation  */
    using DipoleVec = std::array<Dipole, 3>;

    /** @brief Magnetometer Measurement representation
     *          1 LSB = 1e-7 T
     */
    using MagnetometerMeasurement = std::int32_t;

    /** @brief Magnetometer Measurement Vector (3d) representation */
    using MagVec = std::array<MagnetometerMeasurement, 3>;

    /** @brief Gyroscope calibrated Measurement representation
     *          [rad/s]
     */
    using GyroscopeMeasurement = float;

    /** @brief Gyroscope Measurements Vector (3 axes) */
    using GyroVec = std::array<GyroscopeMeasurement, 3>;

    /** @brief Sun Sensor Measurement representation
     *          [rad]
      */
    using SunSensorMeasurement = float;

    /** @brief (Draft) Sun Sensor processed Measurement Vector (2 angles) */
    using SunsVec = std::array<SunSensorMeasurement, 2>;

    /** @} */
}

#endif /* INTERFACE_TYPES_HPP_ */
