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

        /** @brief (Draft)   */
        struct DetumblingConst_Tag
        {
            /** @brief (Draft)   */
            float dt;
            /** @brief (Draft)   */
            float wCutOff;
            /** @brief (Draft)   */
            float bDotGain;
            /** @brief (Draft)   */
            bool coilsOn[3];
        };

        /** @brief (Draft)   */
        struct DetumblingParameters
        {
            /** @brief (Draft)   */
            struct DetumblingConst_Tag DetumblingConst;
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

