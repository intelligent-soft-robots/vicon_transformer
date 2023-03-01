// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief Utility functions for computing transform between point clouds.
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <Eigen/Eigen>

#include "thirdparty/json.hpp"

namespace vicon_transformer
{
using nlohmann::json;

/**
 * @brief Compute mean position error of point cloud transformation.
 *
 * Transform the source points using the given transformation and compute the
 * mean error to the expected target points.
 *
 * @param source_points  Point cloud that is to be transformed.
 * @param expected_target_points  Expected point positions after transformation.
 * @param transform  Transformation that is applied on source_points.
 *
 * @return Mean absolute position error of the transformed source points to
 *      expected_target_points.
 */
double compute_mean_transform_error(Eigen::Matrix3Xd source_points,
                                    Eigen::Matrix3Xd expected_target_points,
                                    Eigen::Isometry3d transform);

/**
 * @brief Extract 3d point cloud in two frames from JSON data structure.
 *
 * The JSON data is expected to be structured as a sequence of objects where
 * each object contains two members with names specified by first_key and
 * second_key.  They are expected to be lists of three numbers each,
 * representing the position of the same point in the first and the second
 * frame.
 *
 * Example (where "foo" and "bar" are first and second key):
 *
 * .. code-block:: json
 *
 *    [
 *      {"foo": [-0.79, 2.11, 0.27], "bar": [-0.54, -0.87, -0.62]},
 *      {"foo": [-0.93, 2.36, 0.26], "bar": [-0.68, -0.62, -0.63]},
 *      ...
 *    ]
 *
 * The positions are extracted and stored in a pair of 3xN matrices.
 *
 * @param json_data JSON structure as described above.
 * @param first_key Name of the entries containing points in the first frame.
 * @param second_key Name of the entries containing points in the second frame.
 *
 * @return Pair of 3xN matrices.  The first element contains all the points
 *      corresponding the first_key, the second element all the points
 *      corresponding to second_key.
 */
std::pair<Eigen::Matrix3Xd, Eigen::Matrix3Xd> json_point_cloud_to_eigen(
    const json &json_data,
    const std::string &first_key,
    const std::string &second_key);

}  // namespace vicon_transformer
