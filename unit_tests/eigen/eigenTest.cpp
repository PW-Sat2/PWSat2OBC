#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"
#include "utils.hpp"

#include <iostream>
#include <Dense>

using testing::_;
using testing::Eq;
using testing::Ne;
using testing::Ge;
using testing::StrEq;
using testing::Return;
using testing::Invoke;
using testing::Pointee;
using testing::ElementsAre;
using testing::Matches;

using Eigen::Matrix3d;
using Eigen::Vector3d;

TEST(eigen_sanity, MatVecCreation)
{
    Matrix3d m = Matrix3d::Identity();
    m = (m + Matrix3d::Constant(1.2)) * 50;
    Vector3d v(1.0, 2.0, 3.0);
    EXPECT_FLOAT_EQ(v[0], 1.0);
    EXPECT_FLOAT_EQ(m(0, 2), 60.0);
    EXPECT_FLOAT_EQ(m(2, 2), 110.0);
}
