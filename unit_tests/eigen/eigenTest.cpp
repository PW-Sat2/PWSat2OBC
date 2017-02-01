#include <iostream>
#include "gtest/gtest.h"

// define custom assert macro for eigen throwing custom type -
//- must be placed before eigen inclusion
typedef int EIGENUT_EXCP_TYPE;
#define eigen_assert(x) if(!(x)){throw EIGENUT_EXCP_TYPE(1);}

#include <Eigen/Dense>

using Eigen::Matrix3f;
using Eigen::Vector3f;


// testing creation of basic types by static allocation
TEST(eigen_sanity, MatVecCreation)
{
  Matrix3f m = Matrix3d::Identity();
  m = (m + Matrix3d::Constant(1.2)) * 50;
  Vector3f v(1.0, 2.0, 3.0);
  EXPECT_FLOAT_EQ(v[0], 1.0);
  EXPECT_FLOAT_EQ(m(0, 2), 60.0);
  EXPECT_FLOAT_EQ(m(2, 2), 110.0);
}

// function using dynamic allocation - must throw exception from
//custom defined eigen_assert function if EIGEN_NO_MALLOC is defined
void throwMallocAssert()
{
  int size = 50;
  // VectorXf is a vector of floats, with dynamic size.
  Eigen::VectorXf u(size), z(size), w(size);
  u = z + w;
}

// test block of dynamic memory allocation  (EIGEN_NO_MALLOC)
TEST(eigen_sanity, NoDynAlloc)
{
  std::cout << "[***INFO***] EigenUT: \"GLib-WARNING\" is expected due to EIGEN_NO_MALLOC testing." << std::endl;

  EXPECT_THROW(throwMallocAssert(), EIGENUT_EXCP_TYPE);
}
