/**
 * @file
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <filesystem>

#include <spdlog/logger.h>
#include <vicon-datastream-sdk/DataStreamClient.h>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>

#include "types.hpp"

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

//! Configuration structure for the ViconReceiver class.
struct ViconReceiverConfig
{
    /**
     * @brief Enable lightweight mode.
     *
     * If enabled, the pose information of the subjects is provided with reduced
     * precision, thus reducing the amount of data that needs to be transmitted.
     * See the Vicon documentation for more information.
     */
    bool enable_lightweight = false;

    /**
     * @brief Buffer size used by the Vicon client.  If set to zero, no buffer
     * is used, i.e. the client always provides the newest frame.
     */
    unsigned int buffer_size = 0;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(enable_lightweight), CEREAL_NVP(buffer_size));
    }
};

/**
 * @brief Base class for ViconFrame receiver classes.
 */
class Receiver
{
public:
    virtual ~Receiver()
    {
    }

    /**
     * @brief Get new frame.  Block if no new frame is available yet.
     *
     * @return The acquired frame.
     */
    virtual ViconFrame read() = 0;
};

/**
 * @brief Receive frames from a running Vicon system.
 *
 * This assumes that a compatible Vicon software (e.g. Vicon Tracker) is set up
 * and running on the specified host.
 */
class ViconReceiver : public Receiver
{
public:
    /**
     * @param host_name Host name or IP address of the Vicon PC.
     * @param config Receiver configuration.
     * @param logger A logger instance used for logging output.  If not set, a
     *      logger with name "ViconReceiver" used.
     */
    ViconReceiver(const std::string& host_name,
                  const ViconReceiverConfig& config,
                  std::shared_ptr<spdlog::logger> logger = nullptr);

    ~ViconReceiver();

    //! Check if connected to a Vicon server.
    bool is_connected() const;

    //! Connect to the Vicon server on the specified host.
    void connect();

    //! Disconnect from the Vicon server.
    void disconnect();

    //! Print some info about the server configuration.
    void print_info() const;

    //! Get a new frame from the Vicon system.
    ViconFrame read() override;

    //! Print detailed latency information.
    void print_latency_info() const;

    /**
     * @brief Only receive data for the listed subjects.
     *
     * If set, pose data is only provided for the listed subjects.  Note that
     * other subjects will still be included in the frame data but their pose
     * will not be set.
     * This can be used to reduce the required bandwidth, if only a few of the
     * subjects are of interest.
     *
     * @param subjects List of subject names.
     */
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
class JsonReceiver : public Receiver
{
public:
    JsonReceiver(const std::filesystem::path& filename);

    //! Return the frame that was loaded from the file.
    ViconFrame read() override;

private:
    ViconFrame frame_;
};

/**
 * @brief Load frames from a recorded file and play it back.
 *
 * To record frames from the live system, use the ``vicon_record`` executable.
 */
class PlaybackReceiver : public Receiver
{
public:
    /**
     * @param filename Path to the recorded file.
     * @param logger A logger instance used for logging output.  If not set, a
     *      logger with name "ViconReceiver" used.
     */
    PlaybackReceiver(const std::filesystem::path& filename,
                     std::shared_ptr<spdlog::logger> logger = nullptr);

    /**
     * @brief Get next frame from the recorded file.
     *
     * @return Next frame from the recorded file.
     * @throws std::out_of_range if end of the recording is reached.
     */
    ViconFrame read() override;

private:
    std::shared_ptr<spdlog::logger> log_;
    std::vector<ViconFrame> tape_;
    size_t tape_index_;
};

}  // namespace vicon_transformer
