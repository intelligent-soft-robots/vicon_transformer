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

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <string.h>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cereal/archives/json.hpp>
#include <vicon_transformer/errors.hpp>
#include <vicon_transformer/vicon_receiver.hpp>

int main(int argc, char* argv[])
{
    auto logger = spdlog::get("root");
    if (!logger)
    {
        logger = spdlog::stderr_color_mt("root");
        // auto log_level = spdlog::level::from_str(config.logger_level);
        logger->set_level(spdlog::level::debug);
    }

    // Program options

    int arg_num = 1;

    std::string host_name = "localhost:801";
    if (argc > 1 && strncmp(argv[arg_num], "--", 2) != 0)
    {
        host_name = argv[arg_num];
        arg_num++;
    }

    vicon_transformer::ViconReceiverConfig config;
    bool only_once = false;
    bool json_output = false;
    std::vector<std::string> filtered_subjects;

    for (int a = arg_num; a < argc; ++a)
    {
        std::string arg = argv[a];
        if (arg == "--help")
        {
            fmt::print(
                "Usage: {} <host name> [options]\n\n"
                "Options:\n"
                " --help\n"
                " --lightweight\n"
                " --subjects <subject name> [<subject name> ...]\n"
                " --once\n",
                " --json\n",
                argv[0]);
            return 0;
        }
        else if (arg == "--lightweight")
        {
            config.enable_lightweight = true;
        }
        else if (arg == "--subjects")
        {
            ++a;
            // assuming no subject name starts with "--"
            while (a < argc)
            {
                if (strncmp(argv[a], "--", 2) == 0)
                {
                    --a;
                    break;
                }
                filtered_subjects.push_back(argv[a]);
                ++a;
            }
        }
        else if (arg == "--once")
        {
            only_once = true;
        }
        else if (arg == "--json")
        {
            json_output = true;
        }
        else
        {
            fmt::print(
                std::cerr, "Failed to understand argument '{}'", argv[a]);
            return 1;
        }
    }

    vicon_transformer::ViconReceiver receiver(host_name, config, logger);
    receiver.connect();
    receiver.filter_subjects(filtered_subjects);
    receiver.print_info();

    fmt::print("\n==============================\n\n");

    //  Loop until a key is pressed
    while (true)
    {
        vicon_transformer::ViconFrame frame = receiver.read();
        receiver.print_latency_info();

        if (json_output)
        {
            cereal::JSONOutputArchive json_out(std::cout);
            frame.serialize(json_out);
        }
        else
        {
            fmt::print("{}\n\n", frame);
        }

        if (only_once)
        {
            break;
        }
    }

    receiver.disconnect();
}
