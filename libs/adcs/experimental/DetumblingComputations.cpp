#include <DetumblingComputations.hpp>
#include <cmath>
#include <cstdint>
#include <system.h>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;

using Eigen::RowVector3f;

using namespace adcs;

DetumblingComputations::State::State(const Parameters& p)
    : mtmDotPrev(Eigen::RowVector3f::Zero()), mtmMeasPrev(Eigen::RowVector3f::Zero()), params(Parameters(p))
{
}

DetumblingComputations::DetumblingComputations() : mtmDotExp(0.0f)
{
}

DetumblingComputations::State DetumblingComputations::initialize(const Parameters& param)
{
    mtmDotExp = exp(-param.wCutOff * param.dt);
    return State(param);
}

DipoleVec DetumblingComputations::step(const MagVec& mgmt_meas, State& state)
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
        commDipoleBdot = mtmDot * (-state.params.bDotGain) / (powf(mgmt_input.norm(), 2));
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

    DipoleVec dipole;
    for (unsigned int i = 0; i < dipole.size(); i++)
    {
        dipole[i] = CastWithSaturation<Dipole>(commDipoleBdot[i]);
    }

    return dipole;
}

template <typename T, typename U> T DetumblingComputations::CastWithSaturation(const U& input)
{
    U upper = static_cast<U>(std::numeric_limits<T>::max());
    U lower = static_cast<U>(std::numeric_limits<T>::min());
    return static_cast<T>(std::max(lower, std::min(input, upper)));
}
