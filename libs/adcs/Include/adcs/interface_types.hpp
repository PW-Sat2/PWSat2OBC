/*
 * interface_types.hpp
 *
 *  Created on: 7 Feb 2017
 *      Author: weclewski
 */

#ifndef INTERFACE_TYPES_HPP_
#define INTERFACE_TYPES_HPP_

#include <Eigen/Dense>

namespace adcs
{
/**
 * @defgroup adcs_types (Draft) ADCS public types
 *
 * @{
 */

/** @brief (Draft) Dipole representaiton
 *          1 LSB = 1e-4 Am^2
 */
using Dipole = std::int16_t;
/** @brief (Draft) Dipole Vector (3d) representaiton  */
using DipoleVec = Eigen::Matrix<Dipole, 3, 1>;

/** @brief (Draft) Magnetometer Measurement representaiton
 *          1 LSB = 1e-7 T
 */
using MagnetometerMeasurement = std::int32_t;
/** @brief (Draft) Magnetometer Measurement Vector (3d) representaiton */
using MagVec = Eigen::Matrix<MagnetometerMeasurement, 3, 1>;

/** @} */
}

#endif /* INTERFACE_TYPES_HPP_ */
