// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief 3D Transformation
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <Eigen/Eigen>

#include <cereal/cereal.hpp>

namespace vicon_transformer
{
/**
 * @brief Represents a 3d transformation.
 *
 * The transformation consists of a rotation R and a translation T with the
 * rotation being applied first.  So the transformed version v' of a vector v is
 * computed as ``v' = R*v + T``.
 */
class Transformation
{
public:
    //! Rotation part of the transformation.
    Eigen::Quaterniond rotation;
    //! Translation part of the transformation.
    Eigen::Vector3d translation;

public:
    //! Construct an identity transformation.
    Transformation();
    //! Construct transformation from the given rotation and translation.
    Transformation(const Eigen::Quaterniond &rotation,
                   const Eigen::Vector3d &translation);
    //! Construct transformation using only a rotation (translation is set to
    //! zero).
    static Transformation fromRotation(const Eigen::Quaterniond &rotation);
    //! Construct transformation using only a translation.
    static Transformation fromTranslation(const Eigen::Vector3d &translation);
    //! Construct an identity transformation.
    static Transformation Identity();

    //! Compose this transformation with the other.
    Transformation operator*(const Transformation &other) const;

    //! Apply the transformation on the given vector (same as @ref apply).
    Eigen::Vector3d operator*(const Eigen::Vector3d &vec) const;
    //! Apply the transformation on the given vector.
    Eigen::Vector3d apply(const Eigen::Vector3d &vec) const;

    //! Invert the transformation.
    Transformation inverse() const;

    //! Convert transformation to a homogeneous matrix (4x4).
    Eigen::Matrix4d matrix() const;

    // for serialisation
    template <class Archive>
    void serialize(Archive &archive)
    {
        archive(cereal::make_nvp("qx", rotation.x()),
                cereal::make_nvp("qy", rotation.y()),
                cereal::make_nvp("qz", rotation.z()),
                cereal::make_nvp("qw", rotation.w()),
                cereal::make_nvp("x", translation.x()),
                cereal::make_nvp("y", translation.y()),
                cereal::make_nvp("z", translation.z()));
    }
};

/**
 * @brief Represents a 3d transformation as translation vector and Euler angles.
 *
 * The convention used for the Euler angles is extrinsic xyz.  Note that these
 * are actually not proper Euler angles but Tait-Bryan (or Cardan) angles.
 * However, the name "Euler" is often used for them as well and probably better
 * known, so we'll stick with this name here.
 */
class EulerTransform
{
public:
    //! Translational part of the transform.
    Eigen::Vector3d translation;
    //! Rotational part of the transform in extrinsic xyz Euler angles [radian].
    Eigen::Vector3d euler_xyz;

    //! Construct identity transformation.
    EulerTransform();

    //! Construct from an isometry transformation.
    EulerTransform(const Eigen::Isometry3d &tf);
};

}  // namespace vicon_transformer
