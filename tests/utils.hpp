/**
 * @file
 * @brief Utility functions, etc. for tests
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <gtest/gtest.h>
#include <Eigen/Eigen>

// Wrapper classes to overwrite how Eigen types are printed by gtest
template <class Base>
class EigenPrintWrap : public Base
{
    friend std::ostream &operator<<(std::ostream &os, const EigenPrintWrap &m)
    {
        os << std::endl << std::endl << static_cast<Base>(m) << std::endl;
        return os;
    }
};
template <class Base>
const EigenPrintWrap<Base> &print_wrap(const Base &base)
{
    return static_cast<const EigenPrintWrap<Base> &>(base);
}

// Quaternion does not implement operator<<, so it needs special handling...
class EigenQuaternionPrintWrap : public Eigen::Quaterniond
{
    friend std::ostream &operator<<(std::ostream &os,
                                    const EigenQuaternionPrintWrap &q)
    {
        fmt::print(
            os, "(w: {}, x: {}, y: {}, z: {})\n", q.w(), q.x(), q.y(), q.z());
        return os;
    }
};
inline const EigenQuaternionPrintWrap &print_wrap(
    const Eigen::Quaterniond &base)
{
    return static_cast<const EigenQuaternionPrintWrap &>(base);
}

// helper for comparing Eigen types with ASSERT_PRED2
template <typename T>
inline bool is_approx(const T &lhs, const T &rhs)
{
    return lhs.isApprox(rhs, 1e-8);
}

// for more convenient use
#define ASSERT_MATRIX_ALMOST_EQUAL(m1, m2) \
    ASSERT_PRED2(is_approx<Eigen::MatrixXd>, print_wrap(m1), print_wrap(m2))

#define ASSERT_QUATERNION_ALMOST_EQUAL(q1, q2) \
    ASSERT_PRED2(is_approx<Eigen::Quaterniond>, print_wrap(q1), print_wrap(q2))
