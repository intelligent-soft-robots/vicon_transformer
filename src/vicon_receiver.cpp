#include <vicon_transformer/vicon_receiver.hpp>

#include <unistd.h>
#include <fstream>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

#include <vicon_transformer/errors.hpp>
#include <vicon_transformer/fmt.hpp>

namespace
{
using Result = ViconDataStreamSDK::CPP::Result::Enum;
}

namespace vicon_transformer
{
std::ostream& operator<<(std::ostream& os, const ViconFrame& vf)
{
    fmt::print(os, "Frame Number: {}\n", vf.frame_number);
    fmt::print(os, "Frame Rate: {}\n", vf.frame_rate);
    fmt::print(os, "Latency: {}\n", vf.latency);
    fmt::print(os, "Timestamp: {}\n", vf.time_stamp);

    fmt::print(os, "Subjects ({}):\n", vf.subjects.size());
    for (auto const& [name, data] : vf.subjects)
    {
        fmt::print(os, "  {}\n", name);
        fmt::print(os, "    Visible: {}\n", data.is_visible);
        fmt::print(os,
                   "    Translation: {}\n",
                   data.global_pose.translation.transpose());
        fmt::print(os,
                   "    Rotation: ({}, {}, {}, {})\n",
                   data.global_pose.rotation.x(),
                   data.global_pose.rotation.y(),
                   data.global_pose.rotation.z(),
                   data.global_pose.rotation.w());
        fmt::print(os, "    Quality: {}\n", data.quality);
    }

    return os;
}

ViconReceiver::ViconReceiver(const std::string& host_name,
                             const ViconReceiverConfig& config,
                             std::shared_ptr<spdlog::logger> logger)
    : host_name_(host_name), config_(config)
{
    if (logger)
    {
        log_ = logger;
    }
    else
    {
        const std::string name = "ViconReceiver";
        if (!(log_ = spdlog::get(name)))
        {
            log_ = spdlog::stderr_color_mt(name);
        }
    }
}

ViconReceiver::~ViconReceiver()
{
    if (is_connected())
    {
        disconnect();
    }
}

bool ViconReceiver::is_connected() const
{
    return client_.IsConnected().Connected;
}

void ViconReceiver::connect()
{
    log_->info("Connecting to {}...", host_name_);
    while (!client_.IsConnected().Connected)
    {
        // Direct connection

        const Result connect_result = client_.Connect(host_name_).Result;
        log_->debug("connect_result = {}", connect_result);
        if (connect_result != Result::Success)
        {
            switch (connect_result)
            {
                case Result::ClientAlreadyConnected:
                    log_->error("Client Already Connected");
                    break;
                case Result::InvalidHostName:
                    log_->error("Invalid Host Name");
                    break;
                case Result::ClientConnectionFailed:
                    log_->error("Client Connection Failed");
                    break;
                default:
                    log_->error("Unrecognized Error: {}", connect_result);
                    break;
            }
            log_->warn("Failed to connect.  Trying again...");
        }

        sleep(1);
    }

    // Enable required data types
    client_.EnableSegmentData();

    if (config_.enable_lightweight)
    {
        log_->info("Enable lightweight segment data.");
        if (client_.EnableLightweightSegmentData().Result != Result::Success)
        {
            throw std::runtime_error(
                "Server does not support lightweight segment data");
        }
    }

    // Set the streaming mode
    client_.SetStreamMode(ViconDataStreamSDK::CPP::StreamMode::ServerPush);

    if (config_.buffer_size > 0)
    {
        log_->info("Set client buffer size to {}", config_.buffer_size);
        client_.SetBufferSize(config_.buffer_size);
    }
}

void ViconReceiver::disconnect()
{
    log_->info("Disconnecting...");
    client_.DisableSegmentData();
    client_.Disconnect();
}

void ViconReceiver::print_info() const
{
    fmt::print("Version: {}\n", client_.GetVersion());

    fmt::print("Segment Data Enabled: {}\n",
               client_.IsSegmentDataEnabled().Enabled);
    fmt::print("Lightweight Segment Data Enabled: {}\n",
               client_.IsLightweightSegmentDataEnabled().Enabled);
    fmt::print("Marker Data Enabled: {}\n",
               client_.IsMarkerDataEnabled().Enabled);
    fmt::print("Unlabeled Marker Data Enabled: {}\n",
               client_.IsUnlabeledMarkerDataEnabled().Enabled);
    fmt::print("Device Data Enabled: {}\n",
               client_.IsDeviceDataEnabled().Enabled);
    fmt::print("Centroid Data Enabled: {}\n",
               client_.IsCentroidDataEnabled().Enabled);
    fmt::print("Marker Ray Data Enabled: {}\n",
               client_.IsMarkerRayDataEnabled().Enabled);
    fmt::print("Centroid Data Enabled: {}\n",
               client_.IsCentroidDataEnabled().Enabled);
    fmt::print("Greyscale Data Enabled: {}\n",
               client_.IsGreyscaleDataEnabled().Enabled);
    fmt::print("Video Data Enabled: {}\n",
               client_.IsVideoDataEnabled().Enabled);
    fmt::print("Debug Data Enabled: {}\n",
               client_.IsDebugDataEnabled().Enabled);

    auto axis_mapping = client_.GetAxisMapping();
    fmt::print("Axis Mapping: X:{} Y:{} Z:{}\n",
               axis_mapping.XAxis,
               axis_mapping.YAxis,
               axis_mapping.ZAxis);
}

ViconFrame ViconReceiver::read()
{
    ViconFrame frame;

    client_get_frame();

    // NOTE: This is only guaranteed to provide a UNIX timestamp
    // starting from C++20!
    // It would actually be better if the timestamp would be provided by
    // the Vicon system itself, but it doesn't seem to have this
    // functionality...
    frame.time_stamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();

    frame.frame_number = client_.GetFrameNumber().FrameNumber;
    frame.frame_rate = client_.GetFrameRate().FrameRateHz;
    frame.latency = client_.GetLatencyTotal().Total;

    // Count the number of subjects
    unsigned int subject_count = client_.GetSubjectCount().SubjectCount;
    for (unsigned int i = 0; i < subject_count; ++i)
    {
        SubjectData subject_data;

        std::string subject_name = client_.GetSubjectName(i).SubjectName;

        // only get pose of root segment
        std::string root_segment =
            client_.GetSubjectRootSegmentName(subject_name).SegmentName;
        auto global_translation =
            client_.GetSegmentGlobalTranslation(subject_name, root_segment);
        auto global_rotation = client_.GetSegmentGlobalRotationQuaternion(
            subject_name, root_segment);

        subject_data.is_visible =
            !(global_translation.Occluded or global_rotation.Occluded);

        // NOTE: Vicon provides quaternion in (x, y, z, w) format but Eigen
        // expects (w, x, y, z).
        const auto& [qx, qy, qz, qw] = global_rotation.Rotation;
        Eigen::Quaterniond rotation(qw, qx, qy, qz);

        // NOTE: Vicon provides translation in millimetres, so needs to be
        // converted to metres
        Eigen::Vector3d translation =
            Eigen::Map<Eigen::Vector3d>(global_translation.Translation) / 1000;

        subject_data.global_pose = Transformation(rotation, translation);

        // Get the quality of the subject (object) if supported
        auto quality = client_.GetObjectQuality(subject_name);
        subject_data.quality =
            (quality.Result == Result::Success) ? quality.Quality : 0.0;

        frame.subjects[subject_name] = subject_data;
    }

    return frame;
}

void ViconReceiver::print_latency_info() const
{
    fmt::print("Latency: {} s\n", client_.GetLatencyTotal().Total);

    for (unsigned int i = 0; i < client_.GetLatencySampleCount().Count; ++i)
    {
        std::string sample_name = client_.GetLatencySampleName(i).Name;
        double sample_value = client_.GetLatencySampleValue(sample_name).Value;

        fmt::print("  {}: {} s\n", sample_name, sample_value);
    }
    fmt::print("\n");
}

void ViconReceiver::filter_subjects(const std::vector<std::string> subjects)
{
    // There needs to be a previously loaded frame in order to add subjects
    // to the filter.  Thus, check if there already is one and try to get
    // a new one if not.
    Result result = client_.GetFrameNumber().Result;
    switch (result)
    {
        case Result::Success:
            break;
        case Result::NoFrame:
            log_->info("Get initial frame before adding subjects to filter.");
            client_get_frame();
            break;
        default:
            throw BadResultError(result);
    }

    for (const std::string& subject_name : subjects)
    {
        log_->info("Add {} to subject filter", subject_name);
        Result result = client_.AddToSubjectFilter(subject_name).Result;
        if (result != Result::Success)
        {
            throw BadResultError(result);
        }
    }
}

void ViconReceiver::client_get_frame()
{
    Result result = client_.GetFrame().Result;

    // verify success
    switch (result)
    {
        case Result::Success:
            return;
        case Result::NotConnected:
            throw NotConnectedError();
        default:
            throw BadResultError(result);
    }
}

JsonReceiver::JsonReceiver(const std::filesystem::path& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error(
            fmt::format("Failed to open file {}", filename));
    }
    frame_ = from_json_stream<ViconFrame>(file);
}

ViconFrame JsonReceiver::read()
{
    return frame_;
}

PlaybackReceiver::PlaybackReceiver(const std::filesystem::path& filename,
                                   std::shared_ptr<spdlog::logger> logger)
{
    if (logger)
    {
        log_ = logger;
    }
    else
    {
        const std::string name = "PlaybackReceiver";
        if (!(log_ = spdlog::get(name)))
        {
            log_ = spdlog::stderr_color_mt(name);
        }
    }

    log_->info("Load Vicon data from file {}", filename);

    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error(
            fmt::format("Failed to open file {}", filename));
    }

    {
        cereal::BinaryInputArchive archive(file);
        archive(tape_);
    }
    tape_index_ = 0;
}

ViconFrame PlaybackReceiver::read()
{
    return tape_.at(tape_index_++);
}

}  // namespace vicon_transformer
