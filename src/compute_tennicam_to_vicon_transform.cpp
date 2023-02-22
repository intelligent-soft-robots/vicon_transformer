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

#include <vicon_transformer/thirdparty/json.hpp>

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

double compute_mean_error(Eigen::MatrixXd tennicam_points,
                          Eigen::MatrixXd vicon_points,
                          Eigen::Isometry3d transform)
{
    Eigen::MatrixXd tc_transformed =
        transform * tennicam_points.colwise().homogeneous();
    Eigen::MatrixXd diff = tc_transformed - vicon_points;
    Eigen::VectorXd norms = diff.colwise().norm();

    return norms.mean();
}

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

    std::ifstream in_stream(args.input_file);
    if (in_stream.fail())
    {
        logger->critical("Could not open file {}.", args.input_file);
        return 1;
    }

    json trajectory;
    try
    {
        in_stream >> trajectory;
    }
    catch (const std::exception &e)
    {
        logger->critical("Failed to load configuration from '{}'. Reason: {}",
                         args.input_file,
                         e.what());
        return 1;
    }

    if (!trajectory.is_array())
    {
        logger->critical("Invalid data structure.  Expected sequence.");
        return 2;
    }

    size_t n_points = trajectory.size();
    logger->info("Loaded trajectory with {} steps.", n_points);

    // load positions from trajectory into Eigen matrices, one column per point
    Eigen::MatrixXd tennicam_points, vicon_points;
    tennicam_points.resize(3, n_points);
    vicon_points.resize(3, n_points);
    for (size_t i = 0; i < n_points; i++)
    {
        std::array<double, 3> tennicam_position, vicon_position;
        trajectory[i].at("tennicam_position").get_to(tennicam_position);
        trajectory[i].at("vicon_position").get_to(vicon_position);

        for (size_t j = 0; j < 3; j++)
        {
            tennicam_points(j, i) = tennicam_position[j];
            vicon_points(j, i) = vicon_position[j];
        }
    }

    logger->debug("tennicam points:\n{}\n",
                  tennicam_points.transpose().format(matrix_list_fmt));
    logger->debug("vicon points:\n{}\n",
                  vicon_points.transpose().format(matrix_list_fmt));

    // compute transformation using Umeyama algorithm
    Eigen::Isometry3d tf;
    tf.matrix() = Eigen::umeyama(tennicam_points, vicon_points, false);
    logger->debug("Transformation matrix:\n{}\n",
                  tf.matrix().format(matrix_list_fmt));

    double mean_error = compute_mean_error(tennicam_points, vicon_points, tf);
    logger->info("Mean error: {}", mean_error);

    // tennicam expects Euler angles in "extrinsic xyz" convention
    Eigen::Vector3d euler_xyz = tf.rotation().eulerAngles(0, 1, 2);

    // print transform in the format used by tennicam_client's config.toml
    fmt::print("[transform]\n");
    fmt::print("translation = {}\n", tf.translation().format(vector_list_fmt));
    fmt::print("# extrinsic xyz Euler angles\n");
    fmt::print("rotation = {}\n", euler_xyz.format(vector_list_fmt));

    return 0;
}
