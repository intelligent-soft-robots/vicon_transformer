// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief Test Transformation
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <gtest/gtest.h>
#include <Eigen/Eigen>

#include <serialization_utils/cereal_json.hpp>

#include <vicon_transformer/transform.hpp>

#include "utils.hpp"

using vicon_transformer::EulerTransform;
using vicon_transformer::Transformation;

TEST(TestTransformation, identity)
{
    auto tf_id = Transformation::Identity();
    Eigen::Vector3d vec(1.0, 2.0, 3.0);
    ASSERT_MATRIX_ALMOST_EQUAL(tf_id.apply(vec), vec);
}

TEST(TestTransformation, initialisation)
{
    Eigen::Quaterniond rot(0.95145453, 0.0948712, 0.29247034, -0.01395812);
    Eigen::Vector3d trans(2.1, -0.2, 0.0);
    auto tf = Transformation(rot, trans);

    ASSERT_QUATERNION_ALMOST_EQUAL(tf.rotation, rot);
    ASSERT_MATRIX_ALMOST_EQUAL(tf.translation, trans);
}

TEST(TestTransformation, rotation)
{
    // note: order is (w, x, y, z)
    Eigen::Quaterniond rot(-0.68456439, 0.37642246, -0.34991817, 0.51694777);
    auto tf = Transformation::fromRotation(rot);
    Eigen::Vector3d vec(1.0, 0, 0);
    Eigen::Vector3d expected(0.22064455, -0.97120219, -0.08990154);

    ASSERT_MATRIX_ALMOST_EQUAL(tf.apply(vec), expected);
    // should also be the same as directly applying the rotation
    ASSERT_MATRIX_ALMOST_EQUAL(rot * vec, expected);
}

TEST(TestTransformation, translation)
{
    auto tf = Transformation::fromTranslation(Eigen::Vector3d(1., 2., 3.));
    Eigen::Vector3d vec(1, 0, -10);
    Eigen::Vector3d expected(2, 2, -7);
    ASSERT_MATRIX_ALMOST_EQUAL(tf.apply(vec), expected);
}

TEST(TestTransformation, full_transformation)
{
    Eigen::Quaterniond rot_x_90(0.70710678, 0., 0., 0.70710678);
    Eigen::Vector3d trans(0, 0, 2);
    auto tf = Transformation(rot_x_90, trans);

    Eigen::Vector3d vec(1, 0, 0);
    Eigen::Vector3d expected(0, 1, 2);
    ASSERT_MATRIX_ALMOST_EQUAL(tf.apply(vec), expected);
    // also test operator*
    ASSERT_MATRIX_ALMOST_EQUAL(tf * vec, expected);
}

TEST(TestTransformation, matrix)
{
    Eigen::Quaterniond rot_x_90(0.70710678, 0., 0., 0.70710678);
    Eigen::Vector3d trans(0.1, 0.2, 0.3);
    auto tf = Transformation(rot_x_90, trans);

    Eigen::Matrix4d expected;
    expected << 0, -1, 0, 0.1, 1, 0, 0, 0.2, 0, 0, 1, 0.3, 0, 0, 0, 1;
    ASSERT_MATRIX_ALMOST_EQUAL(tf.matrix(), expected);
}

TEST(TestTransformation, compose)
{
    auto tf1 =
        Transformation(Eigen::Quaterniond(0.70710678, 0., 0., 0.70710678),
                       Eigen::Vector3d(0.1, 0.2, 0.3));

    auto tf2 = Transformation(
        Eigen::Quaterniond(0.95145453, 0.0948712, 0.29247034, -0.01395812),
        Eigen::Vector3d(2.1, -0.2, 0.0));

    Eigen::Matrix4d mat_comp = tf1.matrix() * tf2.matrix();
    auto tf_comp = tf1 * tf2;

    ASSERT_MATRIX_ALMOST_EQUAL(tf_comp.matrix(), mat_comp);
}

TEST(TestTransformation, inverse)
{
    auto tf = Transformation(
        Eigen::Quaterniond(0.95145453, 0.0948712, 0.29247034, -0.01395812),
        Eigen::Vector3d(2.1, -0.2, 0.0));

    auto inv_tf = tf.inverse();
    Eigen::Matrix4d inv_mat = tf.matrix().inverse();

    ASSERT_MATRIX_ALMOST_EQUAL(inv_tf.matrix(), inv_mat);
}

TEST(TestTransformation, serialize)
{
    auto tf = Transformation(
        Eigen::Quaterniond(0.95145453, 0.0948712, 0.29247034, -0.01395812),
        Eigen::Vector3d(1.0, 2.0, 3.0));

    std::string json = serialization_utils::to_json(tf);
    auto tf2 = serialization_utils::from_json<Transformation>(json);
    ASSERT_QUATERNION_ALMOST_EQUAL(tf.rotation, tf2.rotation);
    ASSERT_MATRIX_ALMOST_EQUAL(tf.translation, tf2.translation);
}

TEST(TestEulerTransform, default_construct)
{
    EulerTransform et;
    // verify everything is set to zero
    ASSERT_TRUE(et.translation.isZero());
    ASSERT_TRUE(et.euler_xyz.isZero());
}

TEST(TestEulerTransform, construct_from_isometry)
{
    // Important: tf needs to be properly initialised (e.g. to identity),
    // otherwise the bottom row of the 4x4 is not initialised by just setting
    // translation and rotation parts below...
    Eigen::Isometry3d tf = Eigen::Isometry3d::Identity();

    Eigen::Vector3d translation(1.3, -0.5, 5.2);
    tf.translation() = translation;
    // matrix constructed from extrinsic xyz Euler angles [0.7, 0.2, -1.3]
    tf.linear() << 0.26216666, 0.77120613, -0.5800946, -0.94435117, 0.08127215,
        -0.3187408, -0.19866933, 0.63137622, 0.74959627;

    EulerTransform et(tf);
    ASSERT_MATRIX_ALMOST_EQUAL(et.translation, translation);
    ASSERT_MATRIX_ALMOST_EQUAL(et.euler_xyz, Eigen::Vector3d(0.7, 0.2, -1.3));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
