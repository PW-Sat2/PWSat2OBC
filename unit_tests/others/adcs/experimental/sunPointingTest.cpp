#include <adcs/SunPointing.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <system.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "adcs/experimental/adcsUtConfig.h"

using adcs::SunPointing;
using adcs::DipoleVec;
using adcs::MagVec;
using adcs::SunsVec;
using adcs::GyroVec;

// cross-validation of detumbling against matlab implementation
TEST(sunpointing, cross_validation)
{
    std::cout << "WARNING: SUNPOINTING TEST... EMPTY" << std::endl;

    // state should be initialized by first measurement
    SunPointing::State state;
    SunPointing::Parameters params;

    SunPointing sp;
    sp.initialize(state, params);

    DipoleVec dipole;
    MagVec mtmMeas;
    bool mtmFlag = false;
    SunsVec ssMeas;
    bool ssFlag = false;
    GyroVec gyrMeas;
    bool gyrFlag = false;
    sp.step(dipole, mtmMeas, mtmFlag, ssMeas, ssFlag, gyrMeas, gyrFlag, state);
}
