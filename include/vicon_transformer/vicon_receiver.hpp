/**
 * @file
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <array>
#include <map>

#include <spdlog/logger.h>
#include <vicon-datastream-sdk/DataStreamClient.h>

namespace vicon_transformer
{
struct SubjectData
{
    bool is_visible;
    std::array<double, 3> global_translation;
    std::array<double, 4> global_rotation_quaternion;
    double quality;
};

struct ViconFrame
{
    // int format_version;
    int frame_number = 0;
    double frame_rate = 0.0;
    double latency = 0.0;
    int64_t time_stamp = 0;

    std::map<std::string, SubjectData> subjects;

    friend std::ostream& operator<<(std::ostream& os, const ViconFrame& vf);
};

struct ViconReceiverConfig
{
    bool enable_lightweight = false;
    unsigned int buffer_size = 0;
};

class ViconReceiver
{
public:
    ViconReceiver(const std::string& host_name,
                  const ViconReceiverConfig& config,
                  std::shared_ptr<spdlog::logger> logger = nullptr);

    ~ViconReceiver();

    bool is_connected() const;

    void connect();

    void disconnect();

    void print_info() const;

    ViconFrame read();

    void print_latency_info() const;

    void filter_subjects(const std::vector<std::string> subjects);

private:
    std::shared_ptr<spdlog::logger> log_;
    ViconDataStreamSDK::CPP::Client client_;

    const std::string host_name_;
    const ViconReceiverConfig config_;

    void client_get_frame();
};
}  // namespace vicon_transformer
