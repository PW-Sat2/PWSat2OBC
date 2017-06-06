#include "SunPointing.hpp"
#include <cmath>
#include <cstdint>
#include <system.h>

using namespace adcs;

SunPointing::SunPointing()
{
}

void SunPointing::initialize(State& state, const Parameters& param)
{
    UNUSED1(state);
    UNUSED1(param);
}

void SunPointing::step(DipoleVec& dipole,
    const MagVec& mtmMeas,
    bool mtmFlag,
    const SunsVec& ssMeas,
    bool ssFlag,
    const GyroVec& gyrMeas,
    bool gyrFlag,
    State& state)
{
    UNUSED1(dipole);
    UNUSED1(mtmMeas);
    UNUSED1(mtmFlag);
    UNUSED1(ssMeas);
    UNUSED1(ssFlag);
    UNUSED1(gyrMeas);
    UNUSED1(gyrFlag);
    UNUSED1(state);
}
