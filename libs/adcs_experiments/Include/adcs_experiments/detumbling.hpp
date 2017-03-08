/*
 * detumbling.hpp
 *
 *  Created on: 25 Jan 2017
 *      Author: weclewski
 */

#ifndef ADCS_DETUMBLING_HPP_
#define ADCS_DETUMBLING_HPP_

#include "interface_types.hpp"
#include <Eigen/Dense>
#include <array>
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::int32_t;
using std::array;

using Eigen::Matrix3d;
using Eigen::Vector3d;
using Eigen::RowVector3f;

namespace adcs
{
/** @brief (Draft)   */
class Detumbling
final
{
    public:
        /** @brief (Draft)   */
       //using RowVector3i = Eigen::Matrix< int32_t , 1, 3 >;
        //using RowVector3b = Eigen::Matrix< bool , 1, 3 >;

        /** @brief (Draft)
         * @unit:
         */
        static constexpr float DefaultDt = 0.2;
        /** @brief (Draft)
         * @unit: [s]
         */
        static constexpr float DefaultWCutOff = 0.2;
        /** @brief (Draft)
         * @unit: [rad/s]
         */
        static constexpr float DefaultBDotGain = 2.879285e-5; /// XXX unit conv
        /** @brief (Draft)
         * @unit: [kg*m^2/s]
         */
        static constexpr std::array <bool, 3> DefaultCoilsOn =
        { true, true, true };

        /** @brief
		* Parameters changeable by a telecommand from Earth. Those are constants used in detumbling.
		*/
        struct DetumblingParameters 
        {
			/**
			*	@detailed: if previous default values would give worse results in detumbling iterative algorithm than commanded from Earth (f.e. better convergence factor) 
			**/
            /** @brief
			* iteration time step, [s]
			*/
            float dt = DefaultDt; 
            /** @brief
			* high-pass filter cut off frequency, [rad/s]
			*/
            float wCutOff = DefaultWCutOff; 
            /** @brief
			* B-dot gain, [kg m^2 / s] (contant k used to calculate commanded magnetic dipole) 
			*/
            float bDotGain = DefaultBDotGain;
            /** @brief
			* active magnetic coils
			*/
            std::array <bool, 3> coilsOn = DefaultCoilsOn; 
        };

        /** @brief (Draft)   */
        struct DetumblingState
        {
            /** @brief (Draft)   */
            RowVector3f mtmDotPrev;
            /** @brief (Draft)   */
            RowVector3f mtmMeasPrev;
            /** @brief (Draft)   */
            //RowVector3f commDipoleBdot_table;
        };

        /** @brief (Draft)   */
        Detumbling();

        /**
         * @brief (Draft)
         * @param[out] state
         * @param[in] parameters
         * @return void
         */
        void initializeDetumbling(DetumblingState& state,
                const DetumblingParameters& param);

        /**
         * @brief (Draft)
         * @param[out] dipole
         * @param[in] magnetometer
         * @param[in/out] state
         * @return void
         */
        void stepDetumbling(DipoleVec& dipole, const MagVec& mgmt,
                DetumblingState& state, const DetumblingParameters& param);
    };
    }

#endif /* ADCS_DETUMBLING_HPP_ */

