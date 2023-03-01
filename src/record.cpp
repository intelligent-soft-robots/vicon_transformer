// SPDX-License-Identifier: BSD-3-Clause

#include <fstream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include <cli_utils/program_options.hpp>

#include <vicon_transformer/vicon_receiver.hpp>

namespace
{
// Class to get console arguments
class Args : public cli_utils::ProgramOptions
{
public:
    std::string host_name;
    std::string out_file;
    double duration_s = 60.0;

    std::string help() const override
    {
        return R"(Record Vicon data and save to file.

Usage:  vicon_record <vicon-host-name> <output-file> [options]

)";
    }

    void add_options(boost::program_options::options_description &options,
                     boost::program_options::positional_options_description
                         &positional) override
    {
        namespace po = boost::program_options;
        // clang-format off
        options.add_options()
            ("vicon-host-name",
             po::value<std::string>(&host_name)->required(),
             "Host name (or IP) of the Vicon PC.")
            ("output-file",
             po::value<std::string>(&out_file)->required(),
             "Path/name of the file to which the recorded data is written.")
            ("duration,d",
             po::value<double>(&duration_s),
             "How long to record (in seconds).  Default: 60 s")
            ;
        // clang-format on

        positional.add("vicon-host-name", 1);
        positional.add("output-file", 1);
    }
};
}  // namespace

int main(int argc, char *argv[])
{
    auto logger = spdlog::get("root");
    if (!logger)
    {
        logger = spdlog::stderr_color_mt("root");
        // auto log_level = spdlog::level::from_str(config.logger_level);
        logger->set_level(spdlog::level::debug);
    }

    // Program options
    Args args;
    if (!args.parse_args(argc, argv))
    {
        return 1;
    }

    std::vector<vicon_transformer::ViconFrame> tape;

    vicon_transformer::ViconReceiverConfig config;
    vicon_transformer::ViconReceiver receiver(args.host_name, config, logger);
    receiver.connect();

    vicon_transformer::ViconFrame frame = receiver.read();
    int64_t duration_ns = args.duration_s * 1e9;
    int64_t end_time = frame.time_stamp + duration_ns;

    logger->info("Start recording for {} s...", args.duration_s);
    while (frame.time_stamp < end_time)
    {
        frame = receiver.read();
        tape.push_back(frame);
    }
    logger->info("End recording");

    {
        logger->info("Safe to file {}", args.out_file);
        std::ofstream file(args.out_file);
        cereal::BinaryOutputArchive json_out(file);
        if (!file.is_open())
        {
            throw std::runtime_error(
                fmt::format("Failed to open file {}", args.out_file));
        }
        json_out(tape);
    }

    receiver.disconnect();

    return 0;
}
