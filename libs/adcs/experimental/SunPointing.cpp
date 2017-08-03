#include "SunPointing.hpp"
#include <cmath>
#include <cstdint>
#include <system.h>

// XXX TO BE REMOVED XXX
#include <iostream>

using namespace adcs;

// XXX TO BE REMOVED XXX
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using Eigen::RowVector3f;
using Eigen::Vector3f;
using Eigen::RowVector2f;
using Eigen::Vector2f;
using Eigen::Matrix;
using RowVector5f = Matrix<float, 1, 5>;
using Vector5f = Matrix<float, 5, 1>;
using Matrix3f = Matrix<float, 3, 3, Eigen::RowMajor>;
using Matrix5f = Matrix<float, 5, 5, Eigen::RowMajor>;

using adcs::SunPointing;
//using adcs::SunPointing::State;

Eigen::Matrix3f skew(const Eigen::Vector3f &vec)
{
    Eigen::Matrix3f ss_matrix = Eigen::Matrix3f::Zero();

    ss_matrix(0, 1) = -vec(2);
    ss_matrix(0, 2) = vec(1);

    ss_matrix(1, 0) = vec(2);
    ss_matrix(1, 2) = -vec(0);

    ss_matrix(2, 0) = -vec(1);
    ss_matrix(2, 1) = vec(0);

    return ss_matrix;
}

/*
 * @brief inverse matrix procedure
 *
 * This procedure calculates the inverse of a [5x5] input matrix. The
 * algorithm performs LU decomposition and then uses Gaussian elimination twice
 * to solve the linear systems.
 * 1. A * X = 1    => X = A^(-1)
 * 2. L*U = A      => L*U * X = 1, where L is lower unit triangular, and U
 *                                 is upper triangular
 * 3. L*Y = 1, solve for each column of Y using Gaussian elimination
 * 4. U*X = Y, solve for each column of X using Gaussian elimination
 * 5. X = A^(-1)
 *
 * Author: Pawel Jaworski
 *         pawel.jaworski0@wp.pl
 * Date:   december 2016
 *
 *   inputs        :
 *      A                - [5x5], input matrix
 *
 *   outputs       :
 *      A_inv            - [5x5], inverse of an input matrix
 *
 *   globals       :
 *
 *   locals        :
 *      n                - [1x1]
 *      L                - [5x5]
 *      U                - [5x5]
 *
 *   subfunctions  :
 *
 *   references    :
 */
/*
Matrix5f matInv(Matrix5f A)
{
    const uint8_t n = 5;

    // LU DECOMPOSITION -------------------------------------------------------
    // initialize
    Matrix5f L = Matrix5f::Identity();
    Matrix5f U = Matrix5f::Zero();
    Matrix5f Y = Matrix5f::Identity();
    Matrix5f AInv = Matrix5f::Zero();
    U(0, 0) = A(0, 0);

    // first row of U and first column of L
    for (uint8_t j = 1; j < n; j++)
    {
        U(0, j) = A(0, j);
        L(j, 0) = A(j, 0) / U(0, 0);
    }

    // up to n-1'th row of U and column of L
    for (uint8_t j = 1; j < n - 1; j++)
    {
        for (uint8_t i = j; i < n; i++)
        {
            U(j, i) = A(j, i);
            for (uint8_t k = 0; k <= j - 1; k++)
            {
                U(j, i) = U(j, i) - L(j, k) * U(k, i);
            }
        }
        for (uint8_t i = j + 1; i < n; i++)
        {
            L(i, j) = A(i, j);
            for (uint8_t k = 0; k <= j - 1; k++)
            {
                L(i, j) = L(i, j) - L(i, k) * U(k, j);
            }
            L(i, j) = L(i, j) / U(j, j);
        }
    }

    // calculate U(n,n)
    uint8_t last = n - 1;
    U(last, last) = A(last, last);
    for (uint8_t k = 0; k < n - 1; k++)
    {
        U(last, last) = U(last, last) - L(last, k) * U(k, last);
    }

    // 1ST GAUSSIAN ELIMINATION -----------------------------------------------
    //XXX?? %solve LY = eye(n) for Y
    for (uint8_t j = 0; j < n - 1; j++)
    {
        Y(j + 1, j) = -L(j + 1, j);
        for (uint8_t i = j + 2; i < n; i++)
        {
            Y(i, j) = -L(i, j);
            for (uint8_t k = j + 1; k < i - 1; k++)
            {
                Y(i, j) = Y(i, j) - L(i, k) * Y(k, j);
            }
        }
    }

    // 2ND GAUSSIAN ELIMINATION -----------------------------------------------
    //XXX?? %solve UX = Y for X. Note that X = A^(-1)

    for (uint8_t j = 0; j < n; j++)
    {
        AInv(last, j) = Y(last, j) / U(last, last);
        for (int8_t i = last - 1; i >= 0; i--)
        {
            AInv(i, j) = Y(i, j);
            for (uint8_t k = i + 1; k < n; k++)
            {
                AInv(i, j) = AInv(i, j) - U(i, k) * AInv(k, j);
            }
            AInv(i, j) = AInv(i, j) / U(i, i);
        }
    }

    return AInv;
}
*/

Matrix5f matInv(Matrix5f A)
{
    return A.inverse();
}

/*
 *                            sun-pointing procedure
 *                          (Attitude Determination)
 *
 * This procedure initializes the Kalman filter. Sun sensor angles and the
 * angular rate is taken directly from the sensors.
 * State covariance matrix is set to the initial state covariance matrix.
 *
 * Author: Pawel Jaworski
 *         pawel.jaworski0@wp.pl
 * Date:   december 2016
 *
 *   inputs        :
 *     ssMeas                        - [2x1], sun sensor latitude & longitude measurements, [rad]
 *     gyroMeas                      - [3x1], gyroscope measurement, [rad/s]
 *
 *   outputs       :
 *     X                             - [5x1], state vector
 *     P                             - [5x5], state covariance matrix
 *     innov                         - [5x1], innovation (difference between actual and predicted measurement)
 *     innovCov                      - [5x5], innovation covariance
 *
 *   globals       :
 *     SunPointingConst.KalmanCov.P0 - [5x5], initial state covariance diagonal matrix
 *     SunPointingConst.KalmanCov.R  - [5x5], measurement covariance diagonal matrix based on the sensors specification
 *
 *   locals        :
 *
 *   subfunctions  :
 *     zeros                         - MATLAB function, creates matrix of zeros with a given size
 *
 *   references    :
 *     Kalman Filter                 - Markley, Crassidis, Fundamentals of Spacecraft Attitude
 *                                     Determination and Control, Springer, 2014, pp. 239-245.
 *                                     doi: 10.1007/978-1-4939-0802-8
 */
void EKFinitialization(Vector5f& xEkf, Matrix5f& pEkf, Vector5f& innov,
        Matrix5f& innovCov, const Vector2f& ssMeas, const Vector3f& gyrMeas,
        const SunPointing::State& state)
{
    // create state vector and set covariance matrix to the initial value
    xEkf << ssMeas, gyrMeas;
    pEkf = Vector5f(state.params.kalmanCov.P0.data()).asDiagonal();
    innov = Vector5f::Zero();
    innovCov = Matrix5f::Zero();
}

/*
 *                            sun-pointing procedure
 *                          (Attitude Determination)
 *
 * This procedure calculates the right hand side function for RK4 integration of
 * Sun sensor angles and angular rate dynamics.
 * State Column Vector x = [lat, long, omegaX, omegaY, omegaZ]' where:
 *
 * Author: Pawel Jaworski
 *         pawel.jaworski0@wp.pl
 * Date:   december 2016
 *
 *   inputs        :
 *     x                            - [5x1], state vector
 *     ctrlTorque                   - [3x1], control torque applied by the coils, [Nm]
 *
 *   outputs       :
 *     xDot                         - [5x1], time derivative of the state vector
 *
 *   globals     :
 *     SunPointingConst.inertia     - [3x3], satellite inertia tensor in body axes, [kg m^2]
 *     SunPointingConst.inertiaInv  - [3x3], inverse of the satellite inertia tensor in body axes, [kg m^2]
 *     SunPointingConst.rotSS       - [3x3], attitude matrix of the sun sensor frame wrt body frame
 *
 *   locals        :
 *     ssLat                        - [1x1]
 *     ssLong                       - [1x1]
 *     angrate                      - [3x1]
 *     sx                           - [1x1]
 *     sy                           - [1x1]
 *     sz                           - [1x1]
 *     sv_SS                        - [3x1]
 *     dLat_dSS                     - [1x3]
 *     htemp                        - [1x1]
 *     hx                           - [1x1]
 *     hy                           - [1x1]
 *     hz                           - [1x1]
 *     dLong_dSS                    - [1x3]
 *     A_b2ss                       - [3x3]
 *     angrate_ss                   - [3x1]
 *     LatDot                       - [1x1]
 *     LongDot                      - [1x1]
 *     angrateDot                   - [3x1]
 *
 *   subfunctions  :
 *     skew                         - calculates [3x3] skew-symmetric matrix of [3x1] vector
 *
 *   references    :
 *     Attitude Dynamics            - Markley, Crassidis, Fundamentals of Spacecraft Attitude
 *                                    Determination and Control, Springer, 2014, pp. 71-111.
 *                                    doi: 10.1007/978-1-4939-0802-8
 */
Vector5f PropagateState(const Vector5f& x, const Vector3f& ctrlTorque,
        const SunPointing::State& state)
{
    float ssLat = x[0];
    float ssLong = x[1];
    Vector3f angrate = x.block(2, 0, 3, 1);

    // Sun Vector in SS frame
    float sx = sinf(ssLat) * cosf(ssLong);
    float sy = sinf(ssLat) * sinf(ssLong);
    float sz = cosf(ssLat);
    Vector3f sv_SS;
    sv_SS << sx, sy, sz;

    // Partial Derivatives
    RowVector3f dLat_dSS;
    dLat_dSS << 0.0f, 0.0f, -1.0f / sqrtf(1.0f - powf(sz, 2.0f));
    float htemp = sy / (sqrtf(1.0f - powf(sz, 2.0f)) + sx);
    float hx = -sy / powf(sqrtf(1.0f - powf(sz, 2.0f)) + sx, 2);
    float hy = 1.0f / (sqrtf(1.0f - powf(sz, 2.0f)) + sx);
    float hz = -hx * sz / sqrtf(1.0f - powf(sz, 2.0f));
    RowVector3f h;
    h << hx, hy, hz;
    RowVector3f dLong_dSS = 2.0f / (powf(htemp, 2.0f) + 1) * h;

    // Angular rate in SS frame
    Matrix3f angrate_ss = Matrix3f(state.params.rotSS.data()) * skew(angrate)
            * Matrix3f(state.params.rotSS.data()).transpose();

    // SS Latitude and Longitude time derivatives
    float LatDot = -dLat_dSS * angrate_ss * sv_SS;
    float LongDot = -dLong_dSS * angrate_ss * sv_SS;

    // angular rate time derivative
    Vector3f angrateDot =
            Matrix3f(state.params.inertiaInv.data())
                    * (ctrlTorque
                            - skew(angrate)
                                    * (Matrix3f(state.params.inertia.data()) * angrate));

    Vector5f xDot;
    xDot << LatDot, LongDot, angrateDot;
    return xDot;
}

/*
 * %                           sun-pointing procedure
 *                          (Attitude Determination)
 *
 * This procedure computes the Sun sensor angles and angular rate estimates
 * given the sun sensor and gyro measurement.
 * Additional outputs, 'res' and 'resCov' are added for performance monitoring.
 *
 * State vector [5x1]:        X = [lat long angrateX angrateY angrateZ]'
 * Measurement Vector [5x1]:  Z = [lat long angrateX angrateY angrateZ]'
 *
 * Author: Pawel Jaworski
 *         pawel.jaworski0@wp.pl
 * Date:   december 2016
 *
 *   inputs        :
 *     xEkfPrev                    - [5x1], previous state vector
 *     pEkf_prev                    - [5x5], previous state covariance matrix
 *     ssMeas                        - [2x1], sun sensor latitude & longitude measurements, [rad]
 *     gyroMeas                      - [3x1], gyroscope measurement, [rad/s]
 *     ssFlag                        - boolean, sun sensor flag
 *     gyroFlag                      - boolean, gyroscope flag
 *     ctrlTorquePrev                - [3x1], previous control torque, [Nm]
 *
 *   outputs       :
 *     xEkf                         - [5x1], state vector
 *     pEkf                         - [5x5], state covariance matrix
 *     innov                         - [5x1], innovation (difference between actual and predicted measurement)
 *     innovCov                      - [5x5], innovation covariance
 *
 *   globals       :
 *     SunPointingConst.dt           - [1x1], iteration time step, [s]
 *     SunPointingConst.KalmanCov.Q  - [5x5], process covariance diagonal matrix
 *     SunPointingConst.KalmanCov.R  - [5x5], measurement covariance diagonal matrix based on the sensors specification
 *     SunPointingConst.inertia      - [3x3], satellite inertia tensor in body axes, [kg m^2]
 *     SunPointingConst.inertiaInv   - [3x3], inverse of the satellite inertia tensor in body axes, [kg m^2]
 *     SunPointingConst.rotSS        - [3x3], attitude matrix of the sun sensor frame wrt body frame
 *
 *   locals        :
 *     k1                            - [5x1]
 *     k2                            - [5x1]
 *     k3                            - [5x1]
 *     k4                            - [5x1]
 *     xPrio                         - [5x1]
 *     ssLat                         - [1x1]
 *     ssLong                        - [1x1]
 *     angrate                       - [3x1]
 *     sx                            - [1x1]
 *     sy                            - [1x1]
 *     sz                            - [1x1]
 *     sv_SS                         - [3x1]
 *     dLat_dSS                      - [1x3]
 *     htemp                         - [1x1]
 *     hx                            - [1x1]
 *     hy                            - [1x1]
 *     hz                            - [1x1]
 *     dLong_dSS                     - [1x3]
 *     A_b2ss                        - [3x3]
 *     angrate_ss                    - [3x3]
 *     dss_dLat                      - [3x1]
 *     dss_dLong                     - [3x1]
 *     ddLat_dLat_dSS                - [1x3]
 *     ddLong_dLat_dSS               - [1x3]
 *     ddLat_dLong_dSS               - [1x3]
 *     ddLong_dLong_dSS              - [1x3]
 *     dLatDot_dLat                  - [1x1]
 *     dLatDot_dLong                 - [1x1]
 *     dLatDot_dOmega                - [1x1]
 *     dLongDot_dLat                 - [1x1]
 *     dLongDot_dLong                - [1x1]
 *     dLongDot_dOmega               - [1x1]
 *     dOmegaDot_dOmega              - [3x1]
 *     jacobianF                     - [5x5]
 *     stateTrans                    - [5x5]
 *     P_prio                        - [5x5]
 *     ssPrio                        - [2x1]
 *     gyroPrio                      - [3x1]
 *     zPrio                         - [5x1]
 *     zMeas                         - [5x1]
 *     H                             - [5x5]
 *     K                             - [5x5]
 *     delta_x                       - [5x1]
 *     jf_m                          - [5x5]
 *
 *   subfunctions  :
 *     skew                          - calculates [3x3] skew-symmetric matrix of [3x1] vector
 *     PropagateState                - calculates the time derivatives of the EKF state vector elements
 *     matrixInv                     - calculates the inverse of a [5x5] matrix
 *     eye                           - MATLAB function, creates identity square matrix with a given size with 1's on the diagonal
 *     zeros                         - MATLAB function, creates matrix of zeros with a given size
 *
 *   references    :
 *     Kalman Filter                 - Markley, Crassidis, Fundamentals of Spacecraft Attitude
 *                                     Determination and Control, Springer, 2014, pp. 239-245.
 *                                     doi: 10.1007/978-1-4939-0802-8
 */
void ExtendedKalmanFilter(Vector5f& xEkf, Matrix5f& pEkf, Vector5f& innov,
        Matrix5f& innovCov, const Vector2f& ssMeas, bool ssFlag,
        const Vector3f& gyrMeas, bool gyrFlag, const SunPointing::State& state)
{
    Matrix3f rotSS = Matrix3f(state.params.rotSS.data());
    Matrix3f rotSSt = Matrix3f(state.params.rotSS.data()).transpose();

    // PREDICTION -------------------------------------------------------------
    // Propagate state vector using RK4
    Vector5f k1 = PropagateState(state.xEkfPrev, state.ctrlTorquePrev, state);
    Vector5f k2 = PropagateState(state.xEkfPrev + 0.5f * state.params.dt * k1,
            state.ctrlTorquePrev, state);
    Vector5f k3 = PropagateState(state.xEkfPrev + 0.5f * state.params.dt * k2,
            state.ctrlTorquePrev, state);
    Vector5f k4 = PropagateState(state.xEkfPrev + state.params.dt * k3,
            state.ctrlTorquePrev, state);
    Vector5f xPrio = state.xEkfPrev
            + state.params.dt / 6.0f * (k1 + 2.0f * k2 + 2.0f * k3 + k4);

    // Extract previous states
    float ssLat = state.xEkfPrev(0, 0);
    float ssLong = state.xEkfPrev(1, 0);
    Vector3f angrate = state.xEkfPrev.block(2, 0, 3, 1);

    // Sun Vector in SS frame
    float sinLat = sinf(ssLat);
    float sinLong = sinf(ssLong);
    float cosLat = cosf(ssLat);
    float cosLong = cosf(ssLong);

    float sx = sinLat * cosLong;
    float sy = sinLat * sinLong;
    float sz = cosLat;
    Vector3f sv_SS;
    sv_SS << sx, sy, sz;

    // Partial Derivatives
    float szFactor = sqrtf(1.0f - powf(sz, 2.0f));
    RowVector3f dLat_dSS;
    dLat_dSS << 0.0f, 0.0f, -1.0f / szFactor;
    float htemp = sy / (szFactor + sx);
    float hx = -sy / powf(szFactor + sx, 2.0f);
    float hy = 1.0f / (szFactor + sx);
    float hz = -hx * sz / szFactor;
    RowVector3f dLong_dSS = 2.0f / (powf(htemp, 2.0f) + 1.0f)
            * (RowVector3f() << hx, hy, hz).finished();

    // Angular rate in SS frame
    Matrix3f angrate_ss = rotSS * skew(angrate) * rotSSt;

    // Sun vector in SS partial derivatives
    Vector3f dss_dLat;
    dss_dLat << cosLat * cosLong,//
            cosLat * sinLong,//
            -sinLat;
    Vector3f dss_dLong;
    dss_dLong << -sinLat * sinLong,//
            sinLat * cosLong,//
            0.0f;

    // Second order partial derivatives (row vectors)
    float sinLat2 = powf(sinLat, 2.0f);
    float tanHalfLong = tanf(ssLong / 2.0f);
    float tanHalfLong2 = powf(tanHalfLong, 2.0f);
    RowVector3f ddLat_dLat_dSS;
    ddLat_dLat_dSS << 0, 0, cosLat / sinLat2; //XXX is this init the same as above?? Vector3f ddLat_dLat_dSS({ 0, 0, cosf(ssLat) / powf(sinf(ssLat), 2) });
    RowVector3f ddLong_dLat_dSS = RowVector3f::Zero();
    RowVector3f ddLat_dLong_dSS;
    ddLat_dLong_dSS
            << tanHalfLong * cosLat / sinLat2,//
            -cosLat / sinLat2,//
            -tanHalfLong / sinLat * (2.0f / sinLat2 - 1.0f);
    RowVector3f ddLong_dLong_dSS;
    ddLong_dLong_dSS
            << -0.5f / sinLat * (1.0f + tanHalfLong2),//
            0.0f,//
            0.5f * cosLat / sinLat2 * (1.0f + tanHalfLong2);

    // Derivatives of the Sun angles time derivatives wrt the state vector elements
    Matrix3f skewRotSStSvSS = skew(rotSSt * sv_SS);
    float dLatDot_dLat = (-ddLat_dLat_dSS * angrate_ss * sv_SS
            - dLat_dSS * angrate_ss * dss_dLat).value();
    float dLatDot_dLong = (-ddLong_dLat_dSS * angrate_ss * sv_SS
            - dLat_dSS * angrate_ss * dss_dLong).value();
    RowVector3f dLatDot_dOmega = dLat_dSS * rotSS * skewRotSStSvSS;
    float dLongDot_dLat = (-ddLat_dLong_dSS * angrate_ss * sv_SS
            - dLong_dSS * angrate_ss * dss_dLat).value();
    float dLongDot_dLong = (-ddLong_dLong_dSS * angrate_ss * sv_SS
            - dLong_dSS * angrate_ss * dss_dLong).value();
    RowVector3f dLongDot_dOmega = dLong_dSS * rotSS * skewRotSStSvSS;

    // Derivative of the omega time derivative wrt omega
    Matrix3f dOmegaDot_dOmega = Matrix3f(state.params.inertiaInv.data())
            * (skew(Matrix3f(state.params.inertia.data()) * angrate)
                    - skew(angrate) * Matrix3f(state.params.inertia.data()));

    // Discrete Jacobian for state transition
    Matrix5f jacobianF;
    jacobianF << dLatDot_dLat,
            dLatDot_dLong,
            dLatDot_dOmega,
            dLongDot_dLat, dLongDot_dLong,
            dLongDot_dOmega,
            Matrix<float, 3, 2>::Zero(),
            dOmegaDot_dOmega;
    //std::cout << jacobianF<<std::endl;
    //jacobianF.block(2, 0, 3, 2) = Matrix<float, 3, 2>::Zero();
    //jacobianF.block(2, 2, 3, 3) = dOmegaDot_dOmega;

    Matrix5f stateTrans = Matrix5f::Identity() + state.params.dt * jacobianF;
    // Prediction of covariance matrix P
    Matrix5f P_prio1 = stateTrans * state.pEkfPrev * stateTrans.transpose();
    Matrix5f P_prio2 = state.params.dt
            * Vector5f(state.params.kalmanCov.Q.data()).asDiagonal();//XXX !!!!!! investigate
    Matrix5f P_prio = P_prio1 + P_prio2;

    // UPDATE -----------------------------------------------------------------
    // Predict and get sun sensor and gyro measurements
    Vector2f ssPrio = xPrio.block(0, 0, 2, 1);
    Vector3f gyroPrio = xPrio.block(2, 0, 3, 1);

    Vector5f zPrio;
    zPrio << ssPrio, gyroPrio;

    Vector2f ssMeasTmp = ssMeas;
    Vector3f gyroMeasTmp = gyrMeas;

    if (!ssFlag)
    {
        ssMeasTmp = ssPrio;
    }

    if (!gyrFlag)
    {
        gyroMeasTmp = gyroPrio;
    }

    Vector5f zMeas;
    zMeas << ssMeasTmp, gyroMeasTmp;

    // Kalman Gain
    innovCov = P_prio + Matrix5f(Vector5f(state.params.kalmanCov.R.data()).asDiagonal());
    Matrix5f K = P_prio * matInv(innovCov);

    if (!ssFlag)
    {
        K.block(0, 0, 5, 2) = Matrix<float, 5, 2>::Zero();
    }

    if (!gyrFlag)
    {
        K.block(0, 2, 5, 3) = Matrix<float, 5, 3>::Zero();
    }

    // Innovation
    innov = zMeas - zPrio;
    Vector5f delta_x = K * innov;

    // Calculate outputs
    xEkf = xPrio + delta_x;
    Matrix5f jf_m = Matrix5f::Identity() - K;
    pEkf = jf_m * P_prio * jf_m.transpose()
            + K * Vector5f(state.params.kalmanCov.R.data()).asDiagonal() * K.transpose(); // Joseph form
}

/*
 *                            sun-pointing procedure
 *                               (Auxilliary)
 *
 * This procedure extracts the Sun vector in the body frame and the
 * satellite angular rate from the EKF state vector.
 *
 * Author: Pawel Jaworski
 *         pawel.jaworski0@wp.pl
 * Date:   december 2016
 *
 *   inputs        :
 *     xEkf                  - [5x1], state vector from EKF
 *
 *   outputs       :
 *     s2s_bodyEst            - [3x1], estimated satellite 2 Sun unit vector in body frame, [-]
 *     angrateEst             - [3x1], estimated satellite angular rate vector in body frame, [rad/s]
 *
 *   globals       :
 *     SunPointingConst.rotSS - [3x3], attitude matrix of the sun sensor frame wrt body frame
 *
 *   locals        :
 *     SS_lat                 - [1x1]
 *     SS_long                - [1x1]
 *     SunVector_SS           - [3x1]
 *
 *   subfunctions  :
 *
 *   references    :
 */
void ExtractSunVandRate(Vector3f& s2s_bodyEst, Vector3f& angrateEst,
        const Vector5f& xEkf, const SunPointing::State& state)
{
    float SS_lat = xEkf(0, 0);
    float SS_long = xEkf(1, 0);
    Vector3f SunVector_SS;
    SunVector_SS << sinf(SS_lat) * cosf(SS_long), sinf(SS_lat) * sinf(SS_long), cosf(
            SS_lat);

    s2s_bodyEst = Matrix3f(state.params.rotSS.data()).transpose()
            * SunVector_SS;
    angrateEst = xEkf.block(2, 0, 3, 1);
}

/*
 * sun-pointing procedure
 *                             (Attitude Control)
 *
 * This procedure calculates the commanded magnetic dipole for the coils in
 * the Sun Pointing mode.
 *
 * Author: Pawel Jaworski
 *         pawel.jaworski0@wp.pl
 * Date:   december 2016
 *
 *  inputs        :
 *    angrateEst                   - [3x1], estimated satellite angular rate vector in body frame, [rad/s]
 *    s2s_bodyEst                  - [3x1], estimated satellite 2 Sun unit vector in body frame, [-]
 *    mtmMeas                      - [3x1], magnetometer measurement, [Gauss]
 *
 *  outputs       :
 *    commDipoleSP                 - [3x1], commanded magnetic dipole, [A m^2]
 *
 *  globals       :
 *    SunPointingConst.inertia     - [3x3], satellite inertia tensor in body axes, [kg m^2]
 *    SunPointingConst.ctrlGains   - structure with Sun Pointing control gains, all scalars, [-]
 *    SunPointingConst.coilsOn     - boolean, [3x1], active magnetic coils
 *    SunPointingConst.spinComm    - [3x1], commanded angular rate vector in body frame, [rad/s]
 *
 *  locals        :
 *    spinCommNorm                 - [1x1]
 *    angrateComm_s                - [3x1]
 *    K_error                      - [3x1]
 *    P_error                      - [1x1]
 *    nutSelect                    - [3x3]
 *    k                            - [1x1]
 *    kp                           - [1x1]
 *    kn                           - [1x1]
 *    ctrltorq                     - [3x1]
 *    bnorm                        - [1x1]
 *
 *  subfunctions  :
 *    vectorNorm                   - calculates [3x1] vector's norm
 *    skew                         - calculates [3x3] skew-symmetric matrix of [3x1] vector
 *
 *  references    :
 *    Control Law                  - PW-SAT2 ADCS Documentation Phase B
 */
void SpinController(Vector3f& commDipoleSP, const Vector3f& angrateEst,
        const Vector3f& s2s_bodyEst, const Vector3f& mtmMeas,
        const SunPointing::State& state)
{
    float spinCommNorm = 0;
    Vector3f angrateComm_s = Vector3f::Zero();
    Vector3f K_error = Vector3f::Zero();
    float P_error = 0;
    Matrix3f nutSelect = Matrix3f::Zero();
    Vector3f ctrltorq = Vector3f::Zero();
    float bnorm = 0;
// Commanded angular rates ------------------------------------------------
    assert(state.params.spinComm.size() == 3);
    spinCommNorm = Vector3f(state.params.spinComm.data()).norm(); //Map<Matrix3f>(state.params.spinComm, 3).norm();//XXX norm/normalize (3)?
    angrateComm_s = spinCommNorm * s2s_bodyEst; // in satellite frame (current orientation)
// Control torque ---------------------------------------------------------
// angular momentum error
    assert(state.params.inertia.size() == 3 * 3);
    K_error = Matrix3f(state.params.inertia.data())
            * (angrateComm_s - angrateEst);

// precession error
    P_error = state.params.inertia[0] * (spinCommNorm - angrateEst(0, 0)); //XXX state.params.inertia[0][0]

// nutation selection matrix
    nutSelect << 0, 0, 0, //
    0, 1, 0, //
    0, 0, 1;
// control law
    ctrltorq = state.params.ctrlGains.k * K_error
            + state.params.ctrlGains.kp * P_error
                    * (Vector3f() << 1.0f, 0.0f, 0.0f).finished()
            - state.params.ctrlGains.kn * nutSelect * angrateEst;
// Commanded magnetic dipole ----------------------------------------------
    bnorm = (mtmMeas * 1e-4f).norm();
    commDipoleSP = skew(mtmMeas * 1e-4f) * ctrltorq / powf(bnorm, 2.0f);

    if (!state.params.coilsOn[0]) // XXX param??
    {
        commDipoleSP[0] = 0;
    }
    else if (!state.params.coilsOn[1])
    {
        commDipoleSP[1] = 0;
    }
    else if (!state.params.coilsOn[2])
    {
        commDipoleSP[2] = 0;
    }
}

SunPointing::SunPointing()
{
}

void SunPointing::initialize(State& state, const Parameters& param)
{
//XXX    % NOTE: t=0 corresponds to sending the TC to start the SP, but the EKF
//        % may not be initialized due to Sun not in the SS FoV

// initialize state with provided parameters
    state = State(param);

    state.xEkfPrev = Vector5f::Zero();
    state.pEkfPrev = Matrix5f::Zero();
    state.ctrlTorquePrev = Vector3f::Zero();
    state.ekfConvCountPrev = param.convCountMin;
}

void SunPointing::step(DipoleVec& dipole, const MagVec& mtmMeas, bool mtmFlag,
        const SunsVec& ssMeas, bool ssFlag, const GyroVec& gyrMeas,
        bool gyrFlag, State& state) //, Lookups* lookups = NULL)
{
    Vector3f inMtmMeas;
    Vector2f inSsMeas;
    Vector3f inGyrMeas;
    Vector3f commDipoleSP;

    assert(inMtmMeas.size() == mtmMeas.size());
    assert(inSsMeas.size() == ssMeas.size());
    assert(inGyrMeas.size() == gyrMeas.size());
    assert(commDipoleSP.size() == dipole.size());

// inputs to eigen
    for (unsigned int i = 0; i < mtmMeas.size(); i++)
    {
        inMtmMeas[i] = mtmMeas[i] / 1e7f;//TODO check conversion - this is high for initial testing purposes
    }

    for (unsigned int i = 0; i < ssMeas.size(); i++)
    {
        inSsMeas[i] = ssMeas[i];
    }

    for (unsigned int i = 0; i < gyrMeas.size(); i++)
    {
        inGyrMeas[i] = gyrMeas[i];
    }

// Set the EKF initialization flag
    bool EKFisInit = !(state.ekfConvCountPrev == state.params.convCountMin); //???

// EKF CONVERGENCE COUNTER LOGIC ------------------------------------------
    uint32_t ekfConvCount = 0;
    if (ssFlag && gyrFlag)
    {
        ekfConvCount = state.ekfConvCountPrev + 1; //TODO do we need prev?
    }
    else
    {
        ekfConvCount = state.ekfConvCountPrev - 1;
    }

// add counter limits
    if (ekfConvCount > state.params.convCountMax)
    {
        ekfConvCount = state.params.convCountMax;
    }
    else if (ekfConvCount < state.params.convCountMin)
    {
        ekfConvCount = state.params.convCountMin;
    }

// check convergence
    bool EKFisConv = (ekfConvCount >= state.params.convThrd);

// EKF AND EKF INITIALIZATION LOGIC ---------------------------------------
    Vector5f xEkf; //state?
    Matrix5f pEkf; //state?
    Vector3f ctrlTorque; //state?
    Vector5f innov; //lookup
    Matrix5f innovCov; //lookup
    Vector3f angrateEst; //local
    Vector3f s2s_bodyEst; //local

    if (!EKFisInit)
    {
        if (!ssFlag || !gyrFlag)
        {
            xEkf = Vector5f::Zero();
            pEkf = Matrix5f::Zero();
            innov = Vector5f::Zero();
            innovCov = Matrix5f::Zero();
            commDipoleSP = Vector3f::Zero();
            ctrlTorque = Vector3f::Zero();
            return;
        }
        else
        {
            EKFinitialization(xEkf, pEkf, innov, innovCov, inSsMeas, inGyrMeas,
                    state); //TODO move to init method
            EKFisInit = true;
        }
    }
    else
    {
        //XXX innov,innovCov - lookups
        //XXX DEBUG
//        std::cout<<inSsMeas<<std::endl;
//        std::cout<<inGyrMeas<<std::endl;
//        std::cout<<inMtmMeas<<std::endl;
        ExtendedKalmanFilter(xEkf, pEkf, innov, innovCov, inSsMeas, ssFlag,
                inGyrMeas, gyrFlag, state);
    }

    ExtractSunVandRate(s2s_bodyEst, angrateEst, xEkf, state);
// CONTROL ----------------------------------------------------------------
    if (!mtmFlag || !EKFisConv)
    {
        commDipoleSP = RowVector3f::Zero();
    }
    else
    {
        SpinController(commDipoleSP, angrateEst, s2s_bodyEst, inMtmMeas, state);
    }

// mtmMeas converted from [Gauss] to [T]
    ctrlTorque = skew(commDipoleSP) * inMtmMeas * 1e-4;

// preserve state
    state.xEkfPrev = xEkf; // TODO Matlab //XXX do we need that - cant store directly in state???
    state.pEkfPrev = pEkf;
    state.ctrlTorquePrev = ctrlTorque;
    state.ekfConvCountPrev = ekfConvCount;

    for(int i = 0; i < commDipoleSP.size(); i++)
    {
        dipole[i] = commDipoleSP[i] * 1e4;
    }
//    std::cout<<"commDipoleSP[0]: " << commDipoleSP[0] <<std::endl;
//    std::cout<<"commDipoleSP[1]: " << commDipoleSP[1] <<std::endl;
//    std::cout<<"commDipoleSP[2]: " << commDipoleSP[2] <<std::endl;
}
