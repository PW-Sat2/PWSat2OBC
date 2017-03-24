#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include <adcs_experiments/detumbling.hpp>
#include <system.h>
#include <cstring>

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

// checking public interfaces
TEST(detumbling, interfaces)
{
    Detumbling::DetumblingState state;
    Detumbling::DetumblingParameters params;

    Detumbling dtb;
    dtb.initializeDetumbling(state, params);

    DipoleVec dipole;
    MagVec mgmt =
    { 100, 200, 300 };

    for (auto i = 0; i < 10; i++)
    {
        dtb.stepDetumbling(dipole, mgmt, state);
    }

    std::cout << dipole[0] << " " << dipole[1] << " " << dipole[2] << std::endl;

    ASSERT_TRUE(false && "Test not implemented!");
}

#include <unistd.h>

// cross-validation of detumbling against matlab implementation
TEST(detumbling, cross_validation)
{
    std::ifstream file("data/bdot_crossvalidation.csv");
    std::vector<float> record;

    /*std::cout << (int) (bool) file << " record: ";
    for (auto x : record)
    {
        std::cout << x << " ";
    }
    std::cout << std::endl;*/

    Detumbling::DetumblingState state;//should be initialised by first measurement
        Detumbling::DetumblingParameters params;
        DipoleVec dipole, dipole_exp;
            MagVec mgmt;

        Detumbling dtb;
        dtb.initializeDetumbling(state, params);

        //while(!file.eof())
        for(int i =0; i<10;i++)
        {
            record = getRecord(file);
            mgmt[0] = record[1];
            mgmt[1] = record[2];
            mgmt[2] = record[3];
            dipole_exp[0] = record[4];
            dipole_exp[1] = record[5];
            dipole_exp[2] = record[6];
            //std::copy(record.begin()+1, record.begin()+4, mgmt);
            //std::copy(record.begin()+4, record.begin()+7, dipole_exp);

            dtb.stepDetumbling(dipole, mgmt, state);

           EXPECT_FLOAT_EQ(dipole[0], dipole_exp[0]);
           EXPECT_FLOAT_EQ(dipole[1], dipole_exp[1]);
           EXPECT_FLOAT_EQ(dipole[2], dipole_exp[2]);

        }



    ASSERT_TRUE(false && "Test not implemented!");
}
