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
 %   params       :
 %       dt       - [1x1], iteration time step, [s]
 %       wCutOff  - [1x1], high-pass filter cut off frequency, [rad/s]
 %       bDotGain - [1x1], B-dot gain, [kg m^2 / s]
 %       coilsOn  - boolean, [3x1], active magnetic coils
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
#include<iostream> //TODO to be removed

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
}

void Detumbling::stepDetumbling(DipoleVec& dipole, const MagVec& mgmt_meas,
        DetumblingState& state, const DetumblingParameters& param) ///TODO units are wrong
{
        //conversion of input LSB = 1e-7T
        std::array<float, 3> tmp;
        std::copy(mgmt_meas.begin(), mgmt_meas.end(), tmp.begin());
        RowVector3f mgmt_input(tmp.data());

        mgmt_input *= 1e-2;//1e-5T as matlab

        // magnetic field time derivative
        RowVector3f mtmDot = exp(-param.wCutOff * param.dt) * state.mtmDotPrev
                + param.wCutOff * (mgmt_input - state.mtmMeasPrev);

        std::cout<<mtmDot<<std::endl;//XXX debug

        // commanded magnetic dipole to coils
        RowVector3f commDipoleBdot = mtmDot * (-param.bDotGain) * 1e-4
                / (powf((mgmt_input * 1e-4).norm(), 2));

        std::cout<<commDipoleBdot<<std::endl;//XXX debug

        // set inavtive dipoles to zero
        for (int i = 0; i < 3; i++)
        {
            if (!param.coilsOn[i])
            {
                commDipoleBdot[i] = 0;
            }
        }

        // store prev values
        state.mtmDotPrev = mtmDot;
        state.mtmMeasPrev = mgmt_input;

        commDipoleBdot *= 1e4;

        // convert to output LSB = 1e-4Am^2
        std::copy(commDipoleBdot.data(),
                commDipoleBdot.data() + commDipoleBdot.size(), dipole.begin());
    
// XXX'0'?

}

}

