/**
 * @file
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <array>
#include <map>

#include <spdlog/logger.h>
#include <vicon-datastream-sdk/DataStreamClient.h>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/map.hpp>

namespace vicon_transformer
{
struct SubjectData
{
    bool is_visible;
    std::array<double, 3> global_translation;
    std::array<double, 4> global_rotation_quaternion;
    double quality;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(is_visible),
                CEREAL_NVP(global_translation),
                CEREAL_NVP(global_rotation_quaternion),
                CEREAL_NVP(quality));
    }
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

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(frame_number),
                CEREAL_NVP(frame_rate),
                CEREAL_NVP(latency),
                CEREAL_NVP(time_stamp),
                CEREAL_NVP(subjects));
    }
};

struct ViconReceiverConfig
{
    bool enable_lightweight = false;
    unsigned int buffer_size = 0;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(enable_lightweight), CEREAL_NVP(buffer_size));
    }
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
