#include <iostream>
#include <gtest/gtest.h>
#include <adcs_experiments/detumbling.hpp>
#include <system.h>

using adcs::Detumbling;
using adcs::DipoleVec;
using adcs::MagVec;

// checking public interfaces
TEST(detumbling, interfaces)
{
    Detumbling::DetumblingState state;
    Detumbling::DetumblingParameters params;

    Detumbling dtb;
    dtb.initializeDetumbling(state, params);

    DipoleVec dipole;
    MagVec mgmt =
    { 1, 2, 3 };

    for (auto i = 0; i < 10; i++)
    {
        dtb.stepDetumbling(dipole, mgmt, state, params);
    }

    std::cout << dipole[0] << " " << dipole[1] << " " << dipole[2] << std::endl;

    ASSERT_TRUE(false && "Test not implemented!");
}

// cross-validation of detumbling against matlab implementation
TEST(detumbling, cross_validation)
{
    ASSERT_TRUE(false && "Test not implemented!");
}
