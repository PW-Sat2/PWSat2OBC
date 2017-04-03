/*
 * detumbling.cpp
 *
 *  Created on: 25 Jan 2017
 *      Author: PWeclewski based on Matlab code by PJaworski
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

Detumbling::Detumbling() :
        mtmDotExp(0.0f)
{
    //empty
}

void Detumbling::initializeDetumbling(DetumblingState& state,
        const DetumblingParameters& param)
{
    // initialize state with provided parameters
    state = DetumblingState(param);
    mtmDotExp = exp(-state.params.wCutOff * state.params.dt);
}

void Detumbling::stepDetumbling(DipoleVec& dipole, const MagVec& mgmt_meas,
        DetumblingState& state)
{
    RowVector3f mgmt_input;

    for (int i = 0; i < mgmt_input.size(); i++)
    {
        mgmt_input[i] = mgmt_meas[i];
    }

    // magnetic field time derivative
    RowVector3f mtmDot = mtmDotExp * state.mtmDotPrev
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
    for(unsigned int i = 0; i < mgmt_meas.size();i++)
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
    for (int i = 0; i < commDipoleBdot.size(); i++)
    {
        if (!state.params.coilsOn[i])
        {
            commDipoleBdot[i] = 0;
        }
    }

    // store prev values
    state.mtmDotPrev = mtmDot;
    state.mtmMeasPrev = mgmt_input;

    for (unsigned int i = 0; i < dipole.size(); i++)
    {
        dipole[i] = commDipoleBdot[i];
    }
}
}

