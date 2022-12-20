/**
 * @file
 * @brief Utility functions, etc. for tests
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <gtest/gtest.h>
#include <Eigen/Eigen>
#include <fmt/format.h>

// helper for ASSERT_PRED2
template<typename T>
inline bool is_approx(const T &lhs, const T &rhs)
{
    return lhs.isApprox(rhs, 1e-8);
}

// for more convenient use
#define ASSERT_MATRIX_ALMOST_EQUAL(m1, m2) \
    ASSERT_PRED2(is_approx<Eigen::MatrixXd>, m1, m2)

#define ASSERT_QUATERNION_ALMOST_EQUAL(q1, q2) \
    ASSERT_PRED2(is_approx<Eigen::Quaterniond>, q1, q2)

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

inline void PrintTo(const Quaterniond &q, std::ostream *os)
{
    *os << fmt::format("\n(w: {}, x: {}, y: {}, z: {})\n", q.w(), q.x(), q.y(), q.z());
}
}  // namespace Eigen
