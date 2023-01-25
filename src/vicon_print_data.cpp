// SPDX-License-Identifier: MIT

//////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2017 Vicon Motion Systems Ltd
// Copyright (c) 2022 Max Planck Gesellschaft
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//////////////////////////////////////////////////////////////////////////////////

#include <vicon-datastream-sdk/DataStreamClient.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cereal/archives/json.hpp>

#include <cli_utils/program_options.hpp>

#include <vicon_transformer/errors.hpp>
#include <vicon_transformer/vicon_receiver.hpp>

namespace
{
// Class to get console arguments
class Args : public cli_utils::ProgramOptions
{
public:
    std::string host_or_file = "localhost:801";
    bool lightweight = false;
    int num_frames = 0;
    bool json_output = false;
    std::vector<std::string> filtered_subjects;

    std::string help() const override
    {
        return R"(Record Vicon data and save to file.

Usage:  vicon_print_data_cpp <vicon-host-name> [options]

)";
    }

    void add_options(boost::program_options::options_description &options,
                     boost::program_options::positional_options_description
                         &positional) override
    {
        namespace po = boost::program_options;
        // clang-format off
        options.add_options()
            ("vicon-host-name-or-file",
             po::value<std::string>(&host_or_file)->required(),
             "Host name (or IP) of the Vicon PC or the path to a recorded file.")
            ("subjects",
             po::value<std::vector<std::string>>(&filtered_subjects)->multitoken(),
             "Only receive data for the listed subjects.")
            ("lightweight",
             "Enable lightweight frames (needs less bandwidth at the cost of lower precision).")
            ("num,n",
             po::value<int>(&num_frames),
             "Only print the specified number of frames.")
            ("json",
             "Produce JSON-formatted output.")
            ;
        // clang-format on

        positional.add("vicon-host-name-or-file", 1);
    }

    // for boolean flags without values, some post-processing is needed
    void postprocess(const boost::program_options::variables_map &args) override
    {
        lightweight = args.count("lightweight") > 0;
        json_output = args.count("json") > 0;
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

    std::unique_ptr<vicon_transformer::Receiver> receiver;

    // if the argument is an existing file, load it for playback, otherwise
    // assume its a hostname and try to connect
    bool use_vicon_receiver = !std::filesystem::exists(args.host_or_file);
    if (use_vicon_receiver)
    {
        // argument is a hostname/IP
        vicon_transformer::ViconReceiverConfig config;
        config.enable_lightweight = args.lightweight;
        config.filtered_subjects = args.filtered_subjects;

        receiver = std::make_unique<vicon_transformer::ViconReceiver>(
            args.host_or_file, config, logger);

        auto ptr =
            static_cast<vicon_transformer::ViconReceiver *>(receiver.get());
        ptr->connect();
        ptr->print_info();
    }
    else
    {
        // argument is a recorded file
        receiver = std::make_unique<vicon_transformer::PlaybackReceiver>(
            args.host_or_file, logger);

        if (args.lightweight)
        {
            logger->warn(
                "Argument --lightweight is ignored when playing back recorded "
                "file.");
        }
        if (!args.filtered_subjects.empty())
        {
            logger->warn(
                "Argument --subjects is ignored when playing back recorded "
                "file.");
        }
    }

    fmt::print("\n==============================\n\n");

    //  Loop until a key is pressed
    for (int i = 0; args.num_frames == 0 || i < args.num_frames; i++)
    {
        // read the frame
        vicon_transformer::ViconFrame frame;
        try
        {
            frame = receiver->read();
        }
        catch (std::out_of_range &)
        {
            logger->info("Reached end of recording.");
            break;
        }

        // print latency info (only when using ViconReceiver)
        if (use_vicon_receiver)
        {
            auto ptr =
                static_cast<vicon_transformer::ViconReceiver *>(receiver.get());
            ptr->print_latency_info();
        }

        // print the frame data
        if (args.json_output)
        {
            cereal::JSONOutputArchive json_out(std::cout);
            frame.serialize(json_out);
        }
        else
        {
            fmt::print("{}\n\n", frame);
        }
    }

    if (use_vicon_receiver)
    {
        auto ptr =
            static_cast<vicon_transformer::ViconReceiver *>(receiver.get());
        ptr->disconnect();
    }
}
