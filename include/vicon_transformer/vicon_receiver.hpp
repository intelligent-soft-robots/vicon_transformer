/**
 * @file
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <array>
#include <filesystem>
#include <map>

#include <spdlog/logger.h>
#include <vicon-datastream-sdk/DataStreamClient.h>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

namespace vicon_transformer
{
// TODO: move to a more appropriate place (maybe serialization_utils?)
template <typename T>
void to_json_stream(T& obj, std::ostream& stream)
{
    cereal::JSONOutputArchive json_out(stream);
    obj.serialize(json_out);
}

template <typename T>
T from_json_stream(std::istream& stream)
{
    T obj;
    {
        cereal::JSONInputArchive json_in(stream);
        obj.serialize(json_in);
    }
    return obj;
}

template <typename T>
std::string to_json(T& obj)
{
    std::stringstream stream;
    to_json_stream(obj, stream);
    return stream.str();
}

template <typename T>
T from_json(const std::string& json_str)
{
    std::stringstream stream(json_str);
    return from_json_stream<T>(stream);
}

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
        int format_version = 3;
        archive(CEREAL_NVP(format_version));
        if (format_version != 3)
        {
            throw std::runtime_error("Invalid input format");
        }

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

class BaseReceiver
{
public:
    virtual ViconFrame read() = 0;
};

class ViconReceiver : public BaseReceiver
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

    ViconFrame read() override;

    void print_latency_info() const;

    void filter_subjects(const std::vector<std::string> subjects);

private:
    std::shared_ptr<spdlog::logger> log_;
    ViconDataStreamSDK::CPP::Client client_;

    const std::string host_name_;
    const ViconReceiverConfig config_;

    void client_get_frame();
};

/**
 * @brief Load a single frame from a JSON file and return it on every read()
 * call. Meant for testing.
 */
class JsonReceiver : public BaseReceiver
{
public:
    JsonReceiver(const std::filesystem::path& filename);

    ViconFrame read() override;

private:
    ViconFrame frame_;
};
}  // namespace vicon_transformer
