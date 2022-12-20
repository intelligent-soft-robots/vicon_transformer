/**
 * @file
 * @brief Utility functions, etc. for tests
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <gtest/gtest.h>
#include <Eigen/Eigen>

// helper for ASSERT_PRED2
inline bool matrix_almost_equal(const Eigen::MatrixXd &lhs, const Eigen::MatrixXd &rhs)
{
    return lhs.isApprox(rhs, 1e-8);
}

// for more convenient use
#define ASSERT_MATRIX_ALMOST_EQUAL(m1, m2) \
    ASSERT_PRED2(matrix_almost_equal, m1, m2)

// PrintTo functions are needed such that gtest prints the actual values of the
// vectors/matrices instead of a hex dump
namespace Eigen
{
inline void PrintTo(const Vector3d &m, std::ostream *os)
{
    *os << std::endl << m.transpose() << std::endl;
}

inline void PrintTo(const Matrix4d &m, std::ostream *os)
{
    *os << std::endl << m << std::endl;
}
}  // namespace Eigen
