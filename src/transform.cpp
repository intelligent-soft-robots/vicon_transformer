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

}  // namespace vicon_transformer
