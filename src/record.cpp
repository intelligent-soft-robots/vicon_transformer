#include <fstream>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include <vicon_transformer/vicon_receiver.hpp>

int main(/*int argc, char* argv[]*/)
{
    auto logger = spdlog::get("root");
    if (!logger)
    {
        logger = spdlog::stderr_color_mt("root");
        // auto log_level = spdlog::level::from_str(config.logger_level);
        logger->set_level(spdlog::level::debug);
    }

    // Program options
    // FIXME make configurable
    std::string host_name = "10.42.2.29";
    std::string out_file = "tape.dat";
    double duration_s = 60.0;

    std::vector<vicon_transformer::ViconFrame> tape;

    vicon_transformer::ViconReceiverConfig config;
    vicon_transformer::ViconReceiver receiver(host_name, config, logger);
    receiver.connect();

    vicon_transformer::ViconFrame frame = receiver.read();
    int64_t duration_ns = duration_s * 1e9;
    int64_t end_time = frame.time_stamp + duration_ns;

    logger->info("Start recording for {} s...", duration_s);
    while (frame.time_stamp < end_time)
    {
        frame = receiver.read();
        tape.push_back(frame);
    }
    logger->info("End recording");

    {
        logger->info("Safe to file {}", out_file);
        std::ofstream file(out_file);
        cereal::BinaryOutputArchive json_out(file);
        if (!file.is_open())
        {
            throw std::runtime_error(
                fmt::format("Failed to open file {}", out_file));
        }
        json_out(tape);
    }

    receiver.disconnect();

    return 0;
}
