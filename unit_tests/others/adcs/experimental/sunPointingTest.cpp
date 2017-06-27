#include <adcs/SunPointing.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <system.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "adcs/experimental/adcsUtConfig.h"
#include "adcs/experimental/Include/adcs/dataFileTools.hpp"

using adcs::SunPointing;
using adcs::DipoleVec;
using adcs::MagVec;
using adcs::SunsVec;
using adcs::GyroVec;

enum ESPDataIdx
{
    t = 0,                   // 1x1
    mtmMeas = 1,             // 3x1
    ssMeas = 4,              // 2x1
    ssMgyroMeas = 6,         // 3x1
    ssMmtmFlag = 9,          // 1x1
    ssMssFlag = 10,          // 1x1
    ssMgyroFlag = 11,        // 1x1
    ssMX_EKF_Prev = 16,      // 5x1
    ssMP_EKF_Prev = 41,      // 5x5
    ssMctrlTorquePrev = 44,  // 3x1
    ssMEKFconvCountPrev = 45,  // 1x1
    ssMcommDipoleSP = 48,    // 3x1
    ssMctrlTorque = 51,      // 3x1
    ssMX_EKF = 56,           // 5x1
    ssMP_EKF = 81,           // 5x5
    ssMinnov = 86,           // 5x1
    ssMinnovCov = 111,       // 5x5
    ssMEKFisInit = 112,      // 1x1
    ssMEKFisConv = 113,      // 1x1
    ssMEKFconvCount = 114    // 1x1

};

// cross-validation of detumbling against matlab implementation
TEST(sunpointing, cross_validation)
{
    std::cout << "SUNPOINTING TEST... EMPTY" << std::endl;

    std::ifstream
    file(
            ADCS_UT_DATA_FILE_PATH "/sunpointing_crossvalidation.csv");
    if (!file)
    {
        std::cerr << "Cannot find data  file!" << std::endl;
        FAIL();
    }
    std::vector<float> record;

    SunPointing::State state; // should be initialised by first measurement
    SunPointing::Parameters params;

    SunPointing sp;
    sp.initialize(state, params);

    // matlab sim is working with different units
    // input: Sim [T] --> OBC [1e-7 T]
    //double input_scale = 1e7;
    // output: Sim [Am2] --> OBC [1e-4 Am2]
    //double output_scale = 1e4;

    while (!file.eof())
    {
        record = dataFileTools::getRecord(file); // TODO change to sunpointing data pool

#ifdef ADCS_DETUMBLIG_DEBUG
        std::cout << "time: " << record[0] << std::endl;
        std::cout << "in0: " << record[1] << " " << input_scale << " " << record[1] * input_scale << " " << (int)(record[1] * input_scale)
        << std::endl;
        std::cout << "in1: " << record[2] << " " << input_scale << " " << record[2] * input_scale << " " << (int)(record[2] * input_scale)
        << std::endl;
        std::cout << "in2: " << record[3] << " " << input_scale << " " << record[3] * input_scale << " " << (int)(record[3] * input_scale)
        << std::endl;
        std::cout << "out0: " << record[4] << " " << output_scale << " " << record[4] * output_scale << " "
        << (int)(record[4] * output_scale) << std::endl;
        std::cout << "out1: " << record[5] << " " << output_scale << " " << record[5] * output_scale << " "
        << (int)(record[5] * output_scale) << std::endl;
        std::cout << "out2: " << record[6] << " " << output_scale << " " << record[6] * output_scale << " "
        << (int)(record[6] * output_scale) << std::endl;
#endif
        DipoleVec dipole;
        MagVec mtmMeas;
        bool mtmFlag = false;
        SunsVec ssMeas;
        bool ssFlag = false;
        GyroVec gyrMeas;
        bool gyrFlag = false;
        sp.step(dipole, mtmMeas, mtmFlag, ssMeas, ssFlag, gyrMeas, gyrFlag,
                state);
        /*
         sp.step(dipole, mgmt, state);

         EXPECT_NEAR(dipole[0], dipole_exp[0], 1.0);
         EXPECT_NEAR(dipole[1], dipole_exp[1], 1.0);
         EXPECT_NEAR(dipole[2], dipole_exp[2], 1.0);
         */
    }
    file.close();
}
