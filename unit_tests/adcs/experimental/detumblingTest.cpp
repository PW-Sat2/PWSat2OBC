#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include <adcs/Detumbling.hpp>
#include <system.h>
#include <cstring>
#include <unistd.h>
#include "adcs/experimental/adcsDetumblingUtConfig.h"

using adcs::Detumbling;
using adcs::DipoleVec;
using adcs::MagVec;

std::vector<float> getRecord(std::istream& str)
{
    std::vector<float> result;
    std::string line, cell;
    std::getline(str, line);
    std::stringstream lineStream(line);
    float cellf;
    while (getline(lineStream, cell, ','))
    {
        std::stringstream cellStream(cell);
        cellStream >> cellf;
        result.push_back(cellf);
    }
    return result;
}

// cross-validation of detumbling against matlab implementation
TEST(detumbling, cross_validation)
{
    std::ifstream file(DETUMBLIG_UT_DATA_FILE_PATH "/bdot_crossvalidation.csv");
    if (!file)
    {
        std::cerr << "Cannot find data  file!" << std::endl;
        FAIL();
    }
    std::vector<float> record;

    Detumbling::State state; //should be initialised by first measurement
    Detumbling::Parameters params;
    DipoleVec dipole, dipole_exp;
    MagVec mgmt;

    Detumbling dtb;
    dtb.initialize(state, params);

    // matlab sim is working with different units
    // input: Sim [T] --> OBC [1e-7 T]
    double input_scale = 1e7;
    // output: Sim [Am2] --> OBC [1e-4 Am2]
    double output_scale = 1e4;

    while(!file.eof())
    {
        record = getRecord(file);
        mgmt[0] = input_scale * record[1];
        mgmt[1] = input_scale * record[2];
        mgmt[2] = input_scale * record[3];
        dipole_exp[0] = output_scale * record[4];
        dipole_exp[1] = output_scale * record[5];
        dipole_exp[2] = output_scale * record[6];

#ifdef ADCS_DETUMBLIG_DEBUG
        std::cout << "time: " << record[0] << std::endl;
        std::cout << "in0: " << record[1] << " " << input_scale << " " << record[1] * input_scale << " " << (int) (record[1] * input_scale) << std::endl;
        std::cout << "in1: " << record[2] << " " << input_scale << " " << record[2] * input_scale << " " << (int) (record[2] * input_scale) << std::endl;
        std::cout << "in2: " << record[3] << " " << input_scale << " " << record[3] * input_scale << " " << (int) (record[3] * input_scale) << std::endl;
        std::cout << "out0: " << record[4] << " " << output_scale << " " << record[4] * output_scale << " " << (int) (record[4] * output_scale) << std::endl;
        std::cout << "out1: " << record[5] << " " << output_scale << " " << record[5] * output_scale << " " << (int) (record[5] * output_scale) << std::endl;
        std::cout << "out2: " << record[6] << " " << output_scale << " " << record[6] * output_scale << " " << (int) (record[6] * output_scale) << std::endl;
#endif

        dtb.step(dipole, mgmt, state);

        EXPECT_NEAR(dipole[0], dipole_exp[0], 1.0);
        EXPECT_NEAR(dipole[1], dipole_exp[1], 1.0);
        EXPECT_NEAR(dipole[2], dipole_exp[2], 1.0);
    }
    file.close();
}
