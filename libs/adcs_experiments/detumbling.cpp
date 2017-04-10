#include "detumbling.hpp"
#include <cmath>
#include <cstdint>
#include <system.h>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

using Eigen::RowVector3f;

using namespace adcs;

Detumbling::Detumbling() :
        mtmDotExp(0.0f)
{
}

void Detumbling::initialize(State& state, const Parameters& param)
{
    // initialize state with provided parameters
    state = State(param);
    mtmDotExp = exp(-state.params.wCutOff * state.params.dt);
}

void Detumbling::step(DipoleVec& dipole, const MagVec& mgmt_meas, State& state)
{
    RowVector3f mgmt_input;

    for (int i = 0; i < mgmt_input.size(); i++)
    {
        mgmt_input[i] = mgmt_meas[i];
    }

    // magnetic field time derivative
    RowVector3f mtmDot = mtmDotExp * state.mtmDotPrev + state.params.wCutOff * (mgmt_input - state.mtmMeasPrev);

    // commanded magnetic dipole to coils
    RowVector3f commDipoleBdot;
    if (!mgmt_input.isZero(0.0))
    {
        commDipoleBdot = mtmDot * (-state.params.bDotGain) / (powf((mgmt_input).norm(), 2));
    }
    else
    {
        commDipoleBdot = RowVector3f::Zero();
    }

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
