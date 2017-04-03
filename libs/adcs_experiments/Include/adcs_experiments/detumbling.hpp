/*
 * detumbling.hpp
 *
 *  Created on: 25 Jan 2017
 *      Author: PWeclewski based on Matlab code by PJaworski
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
/**
 * @defgroup adcs_detumbling  implementaiton of detumbling algorithm
 *
 * @{
 */

/**
 *  @brief Implementation of detumbling  algorithm
 *
 * This procedure calculates the commanded magnetic dipole for the coils in
 * the detumbling mode based on the B-Dot control law. A high-pass filter for
 * the magnetic field time derivative is included.
 * */
class Detumbling
final
{
    public:
        /** @brief default iteration time step
         * @unit [s]
         */
        static constexpr float DefaultDt = 0.2;

        /** @brief default high-pass filter cut off frequency
         * @unit [rad/s]
         */
        static constexpr float DefaultWCutOff = 0.2;

        /** @brief default B-dot gain
         * @unit [kg*m^2/s]
         */
        static constexpr float DefaultBDotGain = 2.879285e-5 * 1e15; // unit conv - original gain * convwersion

        /** @brief default state of flags enabling coils
         * @unit [-]
         */
        static constexpr std::array<bool, 3> DefaultCoilsOn =
        { true, true, true };

        /**
         * @brief Set of detumbling algorithm  parameters
         */
        class DetumblingParameters
        final
        {
            public:
                DetumblingParameters() = default;

                DetumblingParameters(const DetumblingParameters& p) :
                        dt(p.dt),
                        wCutOff(p.wCutOff),
                        bDotGain(p.bDotGain),
                        coilsOn(p.coilsOn)
                {
                }

                /** @brief iteration time step
                 * @unit [s]
                 */
                float dt = DefaultDt;

                /** @brief high-pass filter cut off frequency
                 * @unit [rad/s]
                 */
                float wCutOff = DefaultWCutOff;

                /** @brief B-dot gain
                 * @unit [kg m^2 / s]
                 */
                float bDotGain = DefaultBDotGain;

                /** @brief state of flags enabling coils
                 * @unit [-]
                 */
                std::array<bool, 3> coilsOn = DefaultCoilsOn;
            };

            /**
             * @brief State of detumbling algorithm
             */
            class DetumblingState
            final
            {
                public:
                    DetumblingState() = default;

                    DetumblingState(const DetumblingParameters& p) :
                            params(DetumblingParameters(p))
                    {
                        mtmDotPrev = RowVector3f::Zero();
                        mtmMeasPrev = RowVector3f::Zero();
                    }

                    DetumblingState(const DetumblingState& s) :
                            mtmDotPrev(s.mtmDotPrev),
                            mtmMeasPrev(s.mtmMeasPrev)
                    {
                        (DetumblingParameters&) params = s.params;
                    }

                    DetumblingState& operator=(const DetumblingState& s)

                    {
                        mtmDotPrev = s.mtmDotPrev;
                        mtmMeasPrev = s.mtmMeasPrev;
                        (DetumblingParameters&) params = s.params;
                        return *this;
                    }

                private:
                    /** @brief Value of magnetic field derivative preserved from previous step   */
                    RowVector3f mtmDotPrev;

                    /** @brief Value of magnetometer measurement preserved from previous step   */
                    RowVector3f mtmMeasPrev;

                    /** @brief Set of algorithm parameters   */
                    const DetumblingParameters params;

                    // allowing only algorithm to access state
                    friend class Detumbling;
                };

                Detumbling();

                /**
                 * @brief Detumbling algorithm initialisation function
                 *
                 * This function sould be called before first step of algorithm
                 * and  every time user intend to change parameters
                 *
                 * @param[out] state container
                 * @param[in] parameters set
                 * @return void
                 */
                void initializeDetumbling(DetumblingState& state,
                        const DetumblingParameters& param);

                /**
                 * @brief Detumbling step function
                 *
                 * This function calculates value to be commanded to dipoles
                 * based on magnetometer measurements and preserved state
                 *
                 * @param[out] values to be comanded to dipoles [1e-4 Am2]
                 * @param[in]  magnetometer measurement [1e-7 T]
                 * @param[in/out] state container
                 * @return void
                 */
                void stepDetumbling(DipoleVec& dipole, const MagVec& mgmt,
                        DetumblingState& state);

            private:
                // field to store exp value calculated once on initialisation
                float mtmDotExp;
            };
            /** @} */
            }

#endif /* ADCS_DETUMBLING_HPP_ */

