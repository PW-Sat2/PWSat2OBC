/*
 * detumbling.cpp
 *
 *  Created on: 25 Jan 2017
 *      Author: weclewski
 */

/* MATLAB:
 % ============================================================================
 %
 %                           detumbling procedure
 %
 % This procedure calculates the commanded magnetic dipole for the coils in
 % the detumbling mode based on the B-Dot control law. A high-pass filter for
 % the magnetic field time derivative is included.
 % NOTE: Current magnetic field time derivative output (mtmDot) is the input
 % in the next iteration (mtmDotPrev). Initialize the previous magnetic field
 % time derivative with 0.
 %
 % Author: Pawel Jaworski
 %         pawel.jaworski0@wp.pl
 % Date:   december 2016
 %
 %   inputs        :
 %       mtmMeas                  - [3x1], magnetometer measurement, [Gauss]
 %       mtmMeasPrev              - [3x1], previous (one iteration back) magnetometer measurement, [Gauss]
 %       mtmDotPrev               - [3x1], previous (one iteration back) magnetic field time derivative, [Gauss/s]
 %
 %   outputs       :
 %       commDipoleBdot           - [3x1], commanded magnetic dipole, [A m^2]
 %       mtmDot                   - [3x1], magnetic field time derivative, [Gauss/s]
 %
 %   globals       :
 %       ADCSParameters.DetumblingConst.dt       - [1x1], iteration time step, [s]
 %       ADCSParameters.DetumblingConst.wCutOff  - [1x1], high-pass filter cut off frequency, [rad/s]
 %       ADCSParameters.DetumblingConst.bDotGain - [1x1], B-dot gain, [kg m^2 / s]
 %       ADCSParameters.DetumblingConst.coilsOn  - boolean, [3x1], active magnetic coils
 %
 %   locals        :
 %
 %   subfunctions  :
 %       vectorNorm               - calculates [3x1] vector norm
 %
 %   references    :
 %
 %
 % ============================================================================
 C:

 Lsb of mtmMeas in int32 representation is 1e-9 [T] = 1e-5 [Gauss]
 */

#include "detumbling.hpp"
#include <system.h>
#include <cmath>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

namespace adcs
{

Detumbling::Detumbling()
{
    //empty
}

void Detumbling::initializeDetumbling(DetumblingState& state,
        const DetumblingParameters& param)
{
    UNUSED1(param);
    //Set the previous time derivative of the magnetic field to zeros.
    state.mtmDotPrev = RowVector3f::Zero();
    //Set the previous MTM measurement to zeros,
    state.mtmMeasPrev = RowVector3f::Zero(); // TODO on the first step should be initialised by measurement value

    state.isInitialised = true;
}

void Detumbling::stepDetumbling(DipoleVec& dipole, const MagVec& mgmt_meas,
        DetumblingState& state,  const DetumblingParameters& param)///TODO units are wrong
{
    if (state.isInitialised)
    {
        //conversion of input
        std::array<float, 3> tmp;
        std::copy(mgmt_meas.begin(), mgmt_meas.end(), tmp.begin());
        RowVector3f mgmt_input(tmp.data());

        // magnetic field time derivative
        RowVector3f mtmDot = exp(-param.wCutOff * param.dt) * state.mtmDotPrev +
                param.wCutOff * (mgmt_input - state.mtmMeasPrev);
/*
        RowVector3f mtmDot = state.mtmDotPrev;
        mtmDot *= expf(-param.wCutOff * param.dt);
        RowVector3f mtmDotDiff = mgmt_input - state.mtmMeasPrev;
        mtmDotDiff *= param.wCutOff;
        mtmDot += mtmDotDiff;
*/
        // commanded magnetic dipole to coils
        RowVector3f commDipoleBdot = mtmDot * (-param.bDotGain) * 1e-4 / powf((mgmt_input* 1e-4).norm(), 2);
/*
        RowVector3f commDipoleBdot = mtmDot;
        commDipoleBdot *= -param.bDotGain;
        RowVector3f meas_tmp = mgmt_input;
        meas_tmp *= 1e-4;
        commDipoleBdot *= 1e-4 / powf(meas_tmp.norm(), 2);
*/
        if (!param.coilsOn[0]) // XXX only one???
            commDipoleBdot[0] = 0;
        else if (!param.coilsOn[1])
            commDipoleBdot[1] = 0;
        else if (!param.coilsOn[2])
            commDipoleBdot[2] = 0;

        // store prev values
        state.mtmDotPrev = mtmDot;
        state.mtmMeasPrev = mgmt_input;

        // convert to output
        std::copy(commDipoleBdot.data(),
                commDipoleBdot.data() + commDipoleBdot.size(), dipole.begin());
    }
// XXX'0'?

}

}

