#ifndef ADCS_SUNPOINTING_HPP_
#define ADCS_SUNPOINTING_HPP_

#include <Eigen/Dense>
#include <array>
#include <cmath>
#include "InterfaceTypes.hpp"

namespace adcs
{
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using Eigen::RowVector3f;
using Eigen::Vector3f;
using Eigen::RowVector2f;
using Eigen::Vector2f;
using Eigen::Matrix3f;
using Eigen::Matrix;
using RowVector5f = Matrix<float, 1, 5>;
using Vector5f = Matrix<float, 5, 1>;
using Matrix55f = Matrix<float, 5, 5>;

/**
 * @defgroup adcs_sunpointing  implementaiton of sunpointing algorithm
 *
 * @{
 */

/**
 *  @brief Implementation of sunpointing  algorithm
 *
 * Implementation of sun pointing experiment. Based on given gyroscope, sun
 * sensor and magnetometer measurements and its validity flags experiment
 * produces magnitudes of commands to magnetotorquers to keep satellite
 * pointing the sun
 * */
class SunPointing
final
{
    public:
        /**
         * @brief Set of SunPointing algorithm  parameters
         */
        class Parameters
        final
        {
            public:
                /** @brief iteration time step
                 * @unit [s]
                 * @default 1.0
                 */
                static constexpr float dt = 1.0f;

                /** @brief satellite inertia tensor in body axes
                 * @unit [kg m^2]
                 * @default {{0.015456,  0.000007,  0.000024},
                 {0.000007,  0.014745, -0.000030},
                 {0.000024, -0.000030,  0.006479}}
                 */
                std::array<float, 3 * 3> inertia =
                {
                { 0.015456f, 0.000007f, 0.000024f, //
                        0.000007f, 0.014745f, -0.000030f, //
                        0.000024f, -0.000030f, 0.006479f } };
                /** @brief inverse of the satellite inertia tensor in body axes,
                 * @unit [kg m^2]
                 * @default {{64.70, -0.03,  -0.24},
                 {-0.03, 67.82,   0.31},
                 {-0.24,  0.31, 154.35}}
                 */
                std::array<float, 3 * 3> inertiaInv =
                {
                { 64.70f, -0.03f, -0.24f, //
                        -0.03f, 67.82f, 0.31f, //
                        -0.24f, 0.31f, 154.35f } };
                /** @brief Sun Pointing control gains
                 * @unit [-]
                 * @default {0.004, 0.004, 0.0001}
                 */
                struct
                {
                    float k, kp, kn;
                } ctrlGains =
                { 0.004f, 0.004f, 0.0001f };

                /** @brief active magnetic coils
                 * @unit [-]
                 * @default {true, true, true}
                 */
                std::array<bool, 3> coilsOn =
                {
                { true, true, true } };

                /** @brief commanded angular rate vector in body frame
                 * @unit [rad/s]
                 * @default {5 * M_PI / 180.0, 0, 0}
                 */
                std::array<float, 3> spinComm =
                {
                { 5.0f * M_PI / 180.0f, 0.0f, 0.0f } };

                /** @brief attitude matrix of the sun sensor frame wrt body frame
                 * @unit [-]
                 * @default {{0,        0,       -1},
                 {0.42262,  0.90631,  0},
                 {0.90631, -0.42262,  0}}
                 */
                std::array<float, 3 * 3> rotSS =
                {
                { 0.0f, 0.0f, -1.0f, //
                        0.42262f, 0.90631f, 0.0f, //
                        0.90631f, -0.42262f, 0.0f } };

                /** @brief diagonals of EKF state matrices
                 */
                struct
                {
                    /** @brief process covariance matrix [5x5] diagonal
                     * @unit [-]
                     * @default { 1 * 1e-8, 1 * 1e-8, 10 * 1e-8, 10 * 1e-8, 10 * 1e-8 }
                     */
                    std::array<float, 5> Q =
                    {
                    { 1.0f * 1e-8f, 1.0f * 1e-8f, 10.0f * 1e-8f, 10.0f * 1e-8f,
                            10.0f * 1e-8f } };

                    /** @brief measurement covariance matrix [5x5] diagonal based on the sensors specification
                     * @unit [-]
                     * @default { (0.5 * M_PI / 180) ^ 2, (0.5 * M_PI / 180) ^ 2, (0.3 * M_PI / 180)^ 2, (0.3 * M_PI / 180) ^ 2, (0.3 * M_PI /
                     * 180) ^ 2 }
                     */
                    std::array<float, 5> R =
                    {
                    { powf(0.5f * M_PI / 180.0f, 2.0f), powf(
                            0.5f * M_PI / 180.0f, 2.0f), powf(
                            0.3f * M_PI / 180.0f, 2.0f), powf(
                            0.3f * M_PI / 180.0f, 2.0f), powf(
                            0.3f * M_PI / 180.0f, 2.0f) } };

                    /** @brief initial state covariance matrix [5x5] diagonal
                     *      [0-1]sun sensor std in [rad]
                     *      [2-4]gyro std in [rad/s]
                     * @unit [mix]
                     * @default { (3 * 0.5 * M_PI / 180) ^ 2, (3 * 0.5 * M_PI / 180) ^ 2, (0.3 * M_PI / 180) ^ 2, (0.3 * M_PI / 180) ^ 2, (0.3 *
                     * M_PI / 180) ^ 2 }
                     */
                    std::array<float, 5> P0 =
                    {
                    { powf(3.0f * 0.5f * M_PI / 180.0f, 2.0f), powf(
                            3.0f * 0.5f * M_PI / 180.0f, 2.0f), powf(
                            0.3f * M_PI / 180.0f, 2.0f), powf(
                            0.3f * M_PI / 180.0f, 2.0f), powf(
                            0.3f * M_PI / 180.0f, 2.0f) } };
                } kalmanCov;

                /** @brief max EKF convergence counter value
                 * @unit [-]
                 * @default 15
                 */
                uint8_t convCountMax = 15;

                /** @brief min EKF convergence counter value
                 * @unit [-]
                 * @default 0
                 */
                uint8_t convCountMin = 0;

                /** @brief EKF convergence threshold
                 * @unit [-]
                 * @default 10
                 */
                uint8_t convThrd = 10;
            };

            class Lookups //XXX???
            final
            {
                public:
                    /** @brief innovation (difference between actual and predicted measurement)   */
                    RowVector5f innov;
                    /** @brief innovation covariance   */
                    Matrix55f innovCov;
                };

                /**
                 * @brief State of SunPointing algorithm
                 */
                class State
                final
                {
                    public:
                        State() = default;

                        State(const Parameters& p) :
                                xEkfPrev(Vector5f::Zero()),
                                pEkfPrev(Matrix55f::Zero()),
                                ctrlTorquePrev(Vector3f::Zero()),
                                ekfConvCountPrev(0),
                                params(Parameters(p))
                        {
                        }

                        /** @brief previous state vector from EKF   */
                        Vector5f xEkfPrev;

                        /** @brief previous state covariance matrix   */
                        Matrix55f pEkfPrev;

                        /** @brief previous control torque, [N m]   */
                        Vector3f ctrlTorquePrev;

                        /** @brief previous EKF convergence counter value   */
                        uint32_t ekfConvCountPrev;

                        /** @brief Set of algorithm parameters   */
                        Parameters params;
                    };

                    SunPointing();

                    /**
                     * @brief SunPointing algorithm initialization function
                     *
                     * This function should be called before first step of algorithm
                     * and  every time user intend to change parameters
                     *
                     * @param[out] state container
                     * @param[in] parameters set
                     * @return void
                     */
                    void initialize(State& state, const Parameters& param);

                    /**
                     * @brief SunPointing step function
                     *
                     * This function calculates value to be commanded to dipoles
                     * based on magnetometer, gyroscope and sun sensor measurements,
                     * sensor validity flags and preserved state
                     *
                     * @param[out] values to be commanded to dipoles [1e-4 Am2]
                     * @param[in] mtmMeas magnetometer measurement [1e-7 T]
                     * @param[in] mtmFlag magnetometer validity flag [-]
                     * @param[in] ssMeas sun sensor angles [rad]
                     * @param[in] ssFlag sun sensor validity flag [-]
                     * @param[in] gyrMeas gyroscope calibrated measurement [rad/s]
                     * @param[in] gyrFlag gyroscope validity flag [-]
                     * @param[in/out] state container
                     * @return void
                     */
                    void step(DipoleVec& dipole, const MagVec& mtmMeas,
                            bool mtmFlag, const SunsVec& ssMeas, bool ssFlag,
                            const GyroVec& gyrMeas, bool gyrFlag, State& state);
                };
                /** @} */
                }

#endif /* ADCS_SUNPOINTING_HPP_ */
