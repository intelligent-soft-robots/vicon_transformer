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

/**
 * @brief Information about a subject in a Vicon frame.
 *
 * A "subject" corresponds to an object that is registered in the Vicon Tracker
 * software.
 */
struct SubjectData
{
    /**
     * @brief Whether the subject is visible in the frame.
     *
     * IMPORTANT: If this is false, the values of all other fields of this
     * struct are undefined!
     */
    bool is_visible;
    /**
     * @brief Position of the subject w.r.t. the global origin.
     *
     * This field is only set if @ref is_visible is true.
     */
    std::array<double, 3> global_translation;

    /**
     * @brief Orientation of the subject w.r.t. the global origin.
     *
     * The orientation is given as quaternion (x, y, z, w).
     *
     * This field is only set if @ref is_visible is true.
     */
    std::array<double, 4> global_rotation_quaternion;

    //! Quality measure of the pose estimation.
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

/**
 * @brief All data of a single Vicon frame.
 */
struct ViconFrame
{
    //! Frame sequence number.
    int frame_number = 0;
    //! Frame rate of the Vicon system.
    double frame_rate = 0.0;
    //! Latency of the frame.
    double latency = 0.0;
    //! Time stamp when the frame was acquired.
    int64_t time_stamp = 0;

    /**
     * @brief List of subjects.
     *
     * Note that this list always contains entries for all registered subjects,
     * even if they are not visible in the current frame.  Therefore, always
     * check the ``is_visible`` field of the subject before using its pose.
     */
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
class BaseReceiver
{
public:
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
class ViconReceiver : public BaseReceiver
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
class JsonReceiver : public BaseReceiver
{
public:
    JsonReceiver(const std::filesystem::path& filename);

    //! Return the frame that was loaded from the file.
    ViconFrame read() override;

private:
    ViconFrame frame_;
};
}  // namespace vicon_transformer
