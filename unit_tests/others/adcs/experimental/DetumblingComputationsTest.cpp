#include <adcs/DetumblingComputations.hpp>
#include <cstring>
#include <fstream>
#include <iostream>
#include <system.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Include/adcs/dataFileTools.hpp"
#include "adcs/experimental/adcsUtConfig.h"

using adcs::DetumblingComputations;
using adcs::DipoleVec;
using adcs::MagVec;

// cross-validation of detumbling against matlab implementation
TEST(detumbling, cross_validation)
{
    std::ifstream file(ADCS_UT_DATA_FILE_PATH "/bdot_crossvalidation.csv");
    if (!file)
    {
        std::cerr << "Cannot find data  file!" << std::endl;
        FAIL();
    }
    bool first_record = true;
    std::vector<float> record;

    DetumblingComputations::Parameters params;
    DipoleVec dipole_exp;
    MagVec mgmt;

    DetumblingComputations dtb;
    DetumblingComputations::State state;

    // matlab sim is working with different units
    // input: Sim [Gauss == 1e-4 T] --> OBC [1e-9 T]
    double input_scale = 1e5;
    // output: Sim [Am2] --> OBC [1e-4 Am2]
    double output_scale = 1e4;

    while (!file.eof())
    {
        record = dataFileTools::getRecord(file);
        mgmt[0] = input_scale * record[1];
        mgmt[1] = input_scale * record[2];
        mgmt[2] = input_scale * record[3];
        dipole_exp[0] = output_scale * record[4];
        dipole_exp[1] = output_scale * record[5];
        dipole_exp[2] = output_scale * record[6];

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

        if (first_record)
        {
            state = dtb.initialize(params, mgmt);
            first_record = false;
        }

        auto dipole = dtb.step(mgmt, state);

        EXPECT_NEAR(dipole[0], dipole_exp[0], 1.0);
        EXPECT_NEAR(dipole[1], dipole_exp[1], 1.0);
        EXPECT_NEAR(dipole[2], dipole_exp[2], 1.0);
    }
    file.close();
}

TEST(detumbling, ValueConversions)
{
    DetumblingComputations dtb;

    ASSERT_THAT(dtb.CastWithSaturation<int16_t>(65536.0f), testing::Eq(32767));
    ASSERT_THAT(dtb.CastWithSaturation<int16_t>(-65536.0f), testing::Eq(-32768));
    ASSERT_THAT(dtb.CastWithSaturation<int16_t>(32768.0f), testing::Eq(32767));
    ASSERT_THAT(dtb.CastWithSaturation<int16_t>(-32768.0f), testing::Eq(-32768));
    ASSERT_THAT(dtb.CastWithSaturation<int16_t>(30000.0f), testing::Eq(30000));
    ASSERT_THAT(dtb.CastWithSaturation<int16_t>(-30000.0f), testing::Eq(-30000));
}
