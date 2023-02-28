// SPDX-License-Identifier: BSD-3-Clause

#include <vicon_transformer/transform.hpp>

namespace vicon_transformer
{
Transformation::Transformation()
    : Transformation(Eigen::Quaterniond::Identity(), Eigen::Vector3d::Zero())
{
}

Transformation::Transformation(const Eigen::Quaterniond &rotation,
                               const Eigen::Vector3d &translation)
    : rotation(rotation), translation(translation)
{
}

Transformation Transformation::fromRotation(const Eigen::Quaterniond &rotation)
{
    return Transformation(rotation, Eigen::Vector3d::Zero());
}
Transformation Transformation::fromTranslation(
    const Eigen::Vector3d &translation)
{
    return Transformation(Eigen::Quaterniond::Identity(), translation);
}
Transformation Transformation::Identity()
{
    return Transformation(Eigen::Quaterniond::Identity(),
                          Eigen::Vector3d::Zero());
}

Transformation Transformation::operator*(const Transformation &other) const
{
    Eigen::Quaterniond rot = rotation * other.rotation;
    Eigen::Vector3d trans = translation + rotation * other.translation;
    return Transformation(rot, trans);
}

Eigen::Vector3d Transformation::operator*(const Eigen::Vector3d &vec) const
{
    return apply(vec);
}
Eigen::Vector3d Transformation::apply(const Eigen::Vector3d &vec) const
{
    return rotation * vec + translation;
}

Transformation Transformation::inverse() const
{
    Eigen::Quaterniond inv_rot = rotation.inverse();
    Eigen::Vector3d inv_trans = -(inv_rot * translation);
    return Transformation(inv_rot, inv_trans);
}

Eigen::Matrix4d Transformation::matrix() const
{
    Eigen::Affine3d tf = Eigen::Translation3d(translation) * rotation;
    return tf.matrix();
}

EulerTransform::EulerTransform()
{
    translation.setZero();
    euler_xyz.setZero();
}

EulerTransform::EulerTransform(const Eigen::Isometry3d &tf)
{
    translation = tf.translation();

    // Computation of the correct Tait-Bryan angles is a bit arcane...
    //
    // Wikipedia [1] lists equations to compute different orders of
    // Tait-Bryan angles from a rotation matrix. However, there is no
    // mention on Wikipedia if this is for intrinsic or extrinsic angles.
    // Per trial and error I figured out that it is intrinsic.  We need
    // extrinsic, though.
    //
    // According to [2], extrinsic xyz angles correspond to intrinsic zyx
    // angles in reversed order. So using the equations from Wikipedia for
    // ZYX and reversing the order of alpha, beta, gamma, we get
    //
    // a = atan(R21/R11)
    // b = asin(-R31)
    // c = atan(R32/R33)
    //
    // which are the desired extrinsic xyz angles.
    //
    // [1] en.wikipedia.org/wiki/Euler_angles#Rotation_matrix, 2023-02-23
    // [2]
    // en.wikipedia.org/wiki/Davenport_chained_rotations#Conversion_between_intrinsic_and_extrinsic_rotations,
    // 2023-02-23

    const Eigen::Matrix3d &R = tf.rotation();
    euler_xyz[0] = std::atan2(R(2, 1), R(2, 2));
    euler_xyz[1] = std::asin(-R(2, 0));
    euler_xyz[2] = std::atan2(R(1, 0), R(0, 0));
}

}  // namespace vicon_transformer
