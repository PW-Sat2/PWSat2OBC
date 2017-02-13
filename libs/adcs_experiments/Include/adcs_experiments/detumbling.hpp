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

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::int32_t;

using Eigen::Matrix3d;
using Eigen::Vector3d;

namespace adcs
{
/** @brief (Draft)   */
class Detumbling
final
{
    public:
        /** @brief (Draft)   */
        using RowVector3i = Eigen::Matrix< int32_t , 1, 3 >;
        using RowVector3b = Eigen::Matrix< bool , 1, 3 >;

        /** @brief (Draft)
         * @unit:
         */
        static constexpr float DefaultDt = 0.2;
        /** @brief (Draft)
         * @unit:
         */
        static constexpr float DefaultWCutOff = 0.2;
        /** @brief (Draft)
         * @unit:
         */
        static constexpr float DefaultBDotGain = 2.879285e-5; /// XXX unit conv
        /** @brief (Draft)
         * @unit:
         */
        static constexpr std::array <bool, 3> DefaultCoilsOn =
        { true, true, true };

        /** @brief (Draft)   */
        struct DetumblingConstTag
        {
            /** @brief (Draft)   */
            float dt = DefaultDt;
            /** @brief (Draft)   */
            float wCutOff = DefaultWCutOff;
            /** @brief (Draft)   */
            float bDotGain = DefaultBDotGain;
            /** @brief (Draft)   */
            std::array <bool, 3> coilsOn = DefaultCoilsOn;
        };

        /** @brief (Draft)   */
        struct DetumblingParameters
        {
            /** @brief (Draft)   */
            struct DetumblingConstTag DetumblingConst;
        };

        /** @brief (Draft)   */
        struct DetumblingState
        {
            /** @brief (Draft)   */
            RowVector3i mtmDotPrevInMemory;
            /** @brief (Draft)   */
            RowVector3i mtmMeasPrevInMemory;
            /** @brief (Draft)   */
            RowVector3i commDipoleBdot_table;
        };

        /** @brief (Draft)   */
        Detumbling();

        /**
         * @brief (Draft)
         * @param[out] state
         * @param[in] parameters
         * @return void
         */
        void InitializeDetumbling(DetumblingState& state,
                const DetumblingParameters& parameters);

        /**
         * @brief (Draft)
         * @param[out] dipole
         * @param[in] magnetometer
         * @param[in/out] state
         * @return void
         */
        void DoDetumbling(DipoleVec& dipole, const MagVec& magnetometer,
                DetumblingState& state);
    };
    }

#endif /* ADCS_DETUMBLING_HPP_ */

