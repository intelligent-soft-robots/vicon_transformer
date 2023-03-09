// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief Compute the transform from tennicam to Vicon based on recorded
 * trajectory.
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */

#include <fstream>

#include <fmt/ostream.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <Eigen/Eigen>

#include <cli_utils/program_options.hpp>
#include <spatial_transformation/pointcloud.hpp>
#include <spatial_transformation/transformation.hpp>

#include <spatial_transformation/thirdparty/json.hpp>

using json = nlohmann::json;

namespace
{
// format matrices as nested lists [[1,2,3], [4,5,6], ...]
Eigen::IOFormat matrix_list_fmt(Eigen::StreamPrecision,
                                Eigen::DontAlignCols,
                                ", ",
                                ",\n",
                                "[",
                                "]",
                                "[",
                                "]");

// format vectors as toml-compatible list [1, 2, 3]
Eigen::IOFormat vector_list_fmt(
    Eigen::StreamPrecision, Eigen::DontAlignCols, ", ", ", ", "", "", "[", "]");

// Class to get console arguments
class Args : public cli_utils::ProgramOptions
{
public:
    std::string input_file;
    bool verbose;

    std::string help() const override
    {
        return R"(Compute transform from tennicam to Vicon.

Usage:  compute_tennicam_to_vicon_transform <input-file> [options]

Expects as input a JSON file with a trajectory in both tennicam and Vicon frame.
This trajectory can be recorded with record_tennicam_vicon_trajectory.py.

It computes the transform from tennicam origin to Vicon origin.  With this
tennicam can be configured to transform ball positions to the Vicon frame.

The resulting transform is printed in TOML format that is compatible with
tennicam_client's configuration file.

)";
    }

    void add_options(boost::program_options::options_description &options,
                     boost::program_options::positional_options_description
                         &positional) override
    {
        namespace po = boost::program_options;
        // clang-format off
        options.add_options()
            ("input-file",
             po::value<std::string>(&input_file)->required(),
             "JSON file with the tennicam-vicon trajectory data.")
            ("verbose,v",
             "Enable debug output.")
            ;
        // clang-format on

        positional.add("input-file", 1);
    }

    // for boolean flags without values, some post-processing is needed
    void postprocess(const boost::program_options::variables_map &args) override
    {
        verbose = args.count("verbose") > 0;
    }
};

}  // namespace

int main(int argc, char *argv[])
{
    // Program options
    Args args;
    if (!args.parse_args(argc, argv))
    {
        return 1;
    }

    auto logger = spdlog::get("root");
    if (!logger)
    {
        logger = spdlog::stderr_color_mt("root");
        logger->set_level(args.verbose ? spdlog::level::debug
                                       : spdlog::level::info);
    }

    // load positions from trajectory into Eigen matrices, one column per point
    Eigen::Isometry3d tf;
    double mean_error;
    try
    {
        // load positions from trajectory into Eigen matrices, one column per
        // point
        const auto &[tennicam_points, vicon_points] =
            spatial_transformation::read_point_clouds_from_json_file(
                args.input_file, "tennicam_position", "vicon_position");

        logger->info("Loaded trajectory with {} steps.",
                     tennicam_points.cols());
        logger->debug("tennicam points:\n{}\n",
                      tennicam_points.transpose().format(matrix_list_fmt));
        logger->debug("vicon points:\n{}\n",
                      vicon_points.transpose().format(matrix_list_fmt));

        // compute transformation
        std::tie(tf, mean_error) =
            spatial_transformation::compute_transformation_between_point_clouds(
                tennicam_points, vicon_points);
    }
    catch (const std::exception &e)
    {
        logger->critical(e.what());
        return 1;
    }

    logger->debug("Transformation matrix:\n{}\n",
                  tf.matrix().format(matrix_list_fmt));
    logger->info("Mean error: {}", mean_error);

    // tennicam expects Euler angles in "extrinsic xyz" convention
    spatial_transformation::EulerTransform tennicam_tf(tf);

    // print transform in the format used by tennicam_client's config.toml
    fmt::print("[transform]\n");
    fmt::print("translation = {}\n",
               tennicam_tf.translation.format(vector_list_fmt));
    fmt::print("# extrinsic xyz Euler angles\n");
    fmt::print("rotation = {}\n",
               tennicam_tf.euler_xyz.format(vector_list_fmt));

    return 0;
}
