#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include <adcs_experiments/detumbling.hpp>
#include <system.h>
#include <cstring>
#include <unistd.h>

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
    std::ifstream file("data/bdot_crossvalidation.csv");
    if (!file)
    {
        std::cerr << "Cannot find data  file!" << std::endl;
        FAIL();
    }
    std::vector<float> record;

    Detumbling::DetumblingState state; //should be initialised by first measurement
    Detumbling::DetumblingParameters params;
    DipoleVec dipole, dipole_exp;
    MagVec mgmt;

    Detumbling dtb;
    dtb.initializeDetumbling(state, params);

    double input_scale = 1e9;
    double output_scale = 1e4;

    //while(!file.eof())
    for (int i = 0; i < 10; i++)
    {
        record = getRecord(file);
        mgmt[0] = input_scale * record[1];
        mgmt[1] = input_scale * record[2];
        mgmt[2] = input_scale * record[3];
        dipole_exp[0] = output_scale * record[4];
        dipole_exp[1] = output_scale * record[5];
        dipole_exp[2] = output_scale * record[6];
        //std::copy(record.begin()+1, record.begin()+4, mgmt);
        //std::copy(record.begin()+4, record.begin()+7, dipole_exp);

        if (i == 0)
        {
            state.mtmMeasPrev[0] = mgmt[0];
            state.mtmMeasPrev[1] = mgmt[1];
            state.mtmMeasPrev[2] = mgmt[2];
        }

        dtb.stepDetumbling(dipole, mgmt, state);

        EXPECT_NEAR(dipole[0], dipole_exp[0], 1.0);
        EXPECT_NEAR(dipole[1], dipole_exp[1], 1.0);
        EXPECT_NEAR(dipole[2], dipole_exp[2], 1.0);

        std::cout<<"Comp:"<<std::endl;
        std::cout<<dipole[0]<<" == "<< dipole_exp[0] <<std::endl;
        std::cout<<dipole[1]<<" == "<< dipole_exp[1] <<std::endl;
        std::cout<<dipole[2]<<" == "<< dipole_exp[2] <<std::endl;
    }
    file.close();
}
