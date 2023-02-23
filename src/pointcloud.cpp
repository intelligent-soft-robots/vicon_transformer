// SPDX-License-Identifier: BSD-3-Clause

#include <vicon_transformer/pointcloud.hpp>

namespace vicon_transformer
{
double compute_mean_transform_error(Eigen::Matrix3Xd source_points,
                                    Eigen::Matrix3Xd expected_target_points,
                                    Eigen::Isometry3d transform)
{
    Eigen::Matrix3Xd transformed_points =
        transform * source_points.colwise().homogeneous();
    Eigen::Matrix3Xd diff = transformed_points - expected_target_points;
    Eigen::VectorXd norms = diff.colwise().norm();

    return norms.mean();
}

std::pair<Eigen::Matrix3Xd, Eigen::Matrix3Xd> json_point_cloud_to_eigen(
    const json &json_data,
    const std::string &first_key,
    const std::string &second_key)
{
    if (!json_data.is_array())
    {
        throw std::invalid_argument(
            "Invalid data structure.  Expected sequence.");
    }

    // load positions from trajectory into Eigen matrices, one column per point
    Eigen::Matrix3Xd first_points, second_points;
    const size_t n_points = json_data.size();
    first_points.resize(Eigen::NoChange, n_points);
    second_points.resize(Eigen::NoChange, n_points);
    for (size_t i = 0; i < n_points; i++)
    {
        std::array<double, 3> first_point, second_point;
        json_data[i].at(first_key).get_to(first_point);
        json_data[i].at(second_key).get_to(second_point);

        for (size_t j = 0; j < 3; j++)
        {
            first_points(j, i) = first_point[j];
            second_points(j, i) = second_point[j];
        }
    }

    return std::make_pair(first_points, second_points);
}

}  // namespace vicon_transformer
