/**
 * @file
 * @brief 3D Transformation
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <Eigen/Eigen>

namespace vicon_transformer
{
class Transformation
{
public:
    Eigen::Quaterniond rotation;
    Eigen::Vector3d translation;

public:
    Transformation(const Eigen::Quaterniond &rotation,
                   const Eigen::Vector3d &translation);
    static Transformation fromRotation(const Eigen::Quaterniond &rotation);
    static Transformation fromTranslation(const Eigen::Vector3d &translation);
    static Transformation Identity();

    Transformation operator*(const Transformation &other) const;

    Eigen::Vector3d operator*(const Eigen::Vector3d &vec) const;
    Eigen::Vector3d apply(const Eigen::Vector3d &vec) const;

    Transformation inverse() const;

    Eigen::Matrix4d matrix() const;
};
}  // namespace vicon_transformer
