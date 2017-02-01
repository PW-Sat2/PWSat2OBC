#include <iostream>
#include "gtest/gtest.h"

// define custom assert macro for eigen throwing custom type -
//- must be placed before eigen inclusion
typedef int EIGENUT_EXCP_TYPE;
#define eigen_assert(x) if(!(x)){throw EIGENUT_EXCP_TYPE(1);}

#include <Eigen/Dense>

using Eigen::Matrix3f;
using Eigen::Vector3f;
using Eigen::VectorXf;
using Eigen::Matrix;

// testing creation of basic types by static allocation
TEST(eigen_sanity, MatVecCreation)
{
  Vector3f v(1.0, 2.0, 3.0);
  v = (v + Vector3f::Constant(1.2)) * 50;
  Vector3f v_res;
  v_res << 110.0, 160.0, 210.0;
  EXPECT_TRUE(v.isApprox(v_res));

  Matrix3f m = Matrix3f::Identity();
  m = (m + Matrix3f::Constant(1.2)) * 50;
  Matrix3f m_res;
  m_res <<
  110.0, 60.0, 60.0, //
  60.0, 110.0, 60.0, //
  60.0, 60.0, 110.0;
  EXPECT_TRUE(m.isApprox(m_res));
}

void staticallyAllocatedVector()
{
  using Vector50f = Matrix<float, 50, 1>;
  Vector50f u, z, w;
  u = z + w;
}

// function using dynamic allocation - must throw exception from
//custom defined eigen_assert function if EIGEN_NO_MALLOC is defined
void dynamicallyAllocatedVector()
{
  int size = 50;
  VectorXf u(size), z(size), w(size);
  u = z + w;
}

// test block of dynamic memory allocation  (EIGEN_NO_MALLOC)
TEST(eigen_sanity, NoDynAlloc)
{
  std::cout << "[***INFO***] EigenUT: \"GLib-WARNING\" is expected due to EIGEN_NO_MALLOC testing." << std::endl;
  EXPECT_NO_THROW(staticallyAllocatedVector());
  EXPECT_THROW(dynamicallyAllocatedVector(), EIGENUT_EXCP_TYPE);
}
