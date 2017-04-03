/*
 * detumbling.cpp
 *
 *  Created on: 25 Jan 2017
 *      Author: weclewski
 */

/* BASED ON MATLAB:
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
% Date:   14 june 2016
%
%   inputs      :
%       mtmMeas                  - [3x1], magnetometer measurement, [T]
%       mtmMeasPrev              - [3x1], previous (one iteration back) magnetometer measurement, [T]
%       mtmDotPrev               - [3x1], previous (one iteration back) magnetic field time derivative, [T/s]
%
%   outputs     :
%       commDipoleBdot           - [3x1], commanded magnetic dipole, [A m^2]
%       mtmDot                   - [3x1], magnetic field time derivative, [T/s]
%
%   globals     :
%       DetumblingConst.dt       - scalar, iteration time step, [s]
%       DetumblingConst.wCutOff  - scalar, high-pass filter cut off frequency, [rad/s]
%       DetumblingConst.bDotGain - scalar, B-dot gain, [kg m^2 / s]
%
%   locals      :
%
%   coupling    :
%       vectorNorm               - calculates [3x1] vector norm
%
% ============================================================================
 */

#include "detumbling.hpp"
#include <system.h>
#include <cmath>

#ifdef ADCS_DETUMBLIG_DEBUG
#include<iostream>
#endif

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

namespace adcs
{

// obligatory static definition
constexpr std::array<bool, 3> Detumbling::DefaultCoilsOn;

Detumbling::Detumbling()
{
    //empty
}

void Detumbling::initializeDetumbling(DetumblingState& state,
        const DetumblingParameters& param)
{
    // initialize state with provided parameters
    state = DetumblingState(param);
}

void Detumbling::stepDetumbling(DipoleVec& dipole, const MagVec& mgmt_meas,
        DetumblingState& state)
{
    RowVector3f mgmt_input;

    for (int i = 0; i < 3; i++)
    {
        mgmt_input[i] = mgmt_meas[i];
    }

    // magnetic field time derivative
    RowVector3f mtmDot = exp(-state.params.wCutOff * state.params.dt)
            * state.mtmDotPrev
            + state.params.wCutOff * (mgmt_input - state.mtmMeasPrev);

    // commanded magnetic dipole to coils
    RowVector3f commDipoleBdot;
    if (!mgmt_input.isZero(0.0))
    {
        commDipoleBdot = mtmDot * (-state.params.bDotGain)
                / (powf((mgmt_input).norm(), 2));
    }
    else
    {
        commDipoleBdot = RowVector3f::Zero();
    }

#ifdef ADCS_DETUMBLIG_DEBUG
    std::cout << "mgmt_meas: ";
    for(int i =0; i<3;i++)
    {
        std::cout << mgmt_meas[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "mgmt_input: ";
    std::cout << mgmt_input << std::endl;
    std::cout << "exp: ";
    std::cout << exp(-state.params.wCutOff * state.params.dt) << std::endl;
    std::cout << "state.mtmDotPrev: ";
    std::cout << state.mtmDotPrev << std::endl;
    std::cout << "state.mtmMeasPrev: ";
    std::cout << state.mtmMeasPrev << std::endl;
    std::cout << "mtmDot: ";
    std::cout << mtmDot << std::endl;
    std::cout << "commDipoleBdot: ";
    std::cout << commDipoleBdot << std::endl;
#endif

    // set inavtive dipoles to zero
    for (int i = 0; i < 3; i++)
    {
        if (!state.params.coilsOn[i])
        {
            commDipoleBdot[i] = 0;
        }
    }

    // store prev values
    state.mtmDotPrev = mtmDot;
    state.mtmMeasPrev = mgmt_input;

    for (int i = 0; i < 3; i++)
    {
        dipole[i] = commDipoleBdot[i];
    }
}
}

