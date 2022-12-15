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

#include <array>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>  // For sleep()

#include <string.h>
#include <time.h>

#include <fmt/ostream.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace ViconDataStreamSDK::CPP;

namespace
{
std::string to_string(const bool i_Value)
{
    return i_Value ? "True" : "False";
}

template <typename T>
std::string to_string(const T& x)
{
    std::stringstream ss;
    ss << x;
    return ss.str();
}

template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr)
{
    std::copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(os, " "));
    return os;
}

std::ostream& operator<<(std::ostream& os, const Output_GetVersion& x)
{
    os << x.Major << "." << x.Minor << "." << x.Point << "." << x.Revision;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Direction::Enum& dir)
{
    switch (dir)
    {
        case Direction::Forward:
            os << "Forward";
            break;
        case Direction::Backward:
            os << "Backward";
            break;
        case Direction::Left:
            os << "Left";
            break;
        case Direction::Right:
            os << "Right";
            break;
        case Direction::Up:
            os << "Up";
            break;
        case Direction::Down:
            os << "Down";
            break;
        default:
            os << "Unknown";
            break;
    }

    return os;
}

std::ostream& operator<<(std::ostream& os, const Result::Enum& result)
{
    switch (result)
    {
        case Result::Unknown:
            os << "Unknown";
            break;
        case Result::NotImplemented:
            os << "NotImplemented";
            break;
        case Result::Success:
            os << "Success";
            break;
        case Result::InvalidHostName:
            os << "InvalidHostName";
            break;
        case Result::InvalidMulticastIP:
            os << "InvalidMulticastIP";
            break;
        case Result::ClientAlreadyConnected:
            os << "ClientAlreadyConnected";
            break;
        case Result::ClientConnectionFailed:
            os << "ClientConnectionFailed";
            break;
        case Result::ServerAlreadyTransmittingMulticast:
            os << "ServerAlreadyTransmittingMulticast";
            break;
        case Result::ServerNotTransmittingMulticast:
            os << "ServerNotTransmittingMulticast";
            break;
        case Result::NotConnected:
            os << "NotConnected";
            break;
        case Result::NoFrame:
            os << "NoFrame";
            break;
        case Result::InvalidIndex:
            os << "InvalidIndex";
            break;
        case Result::InvalidCameraName:
            os << "InvalidCameraName";
            break;
        case Result::InvalidSubjectName:
            os << "InvalidSubjectName";
            break;
        case Result::InvalidSegmentName:
            os << "InvalidSegmentName";
            break;
        case Result::InvalidMarkerName:
            os << "InvalidMarkerName";
            break;
        case Result::InvalidDeviceName:
            os << "InvalidDeviceName";
            break;
        case Result::InvalidDeviceOutputName:
            os << "InvalidDeviceOutputName";
            break;
        case Result::InvalidLatencySampleName:
            os << "InvalidLatencySampleName";
            break;
        case Result::CoLinearAxes:
            os << "CoLinearAxes";
            break;
        case Result::LeftHandedAxes:
            os << "LeftHandedAxes";
            break;
        case Result::HapticAlreadySet:
            os << "HapticAlreadySet";
            break;
        case Result::EarlyDataRequested:
            os << "EarlyDataRequested";
            break;
        case Result::LateDataRequested:
            os << "LateDataRequested";
            break;
        case Result::InvalidOperation:
            os << "InvalidOperation";
            break;
        case Result::NotSupported:
            os << "NotSupported";
            break;
        case Result::ConfigurationFailed:
            os << "ConfigurationFailed";
            break;
        case Result::NotPresent:
            os << "NotPresent";
            break;
    }
    return os;
}

}  // namespace

class NotConnectedError : public std::runtime_error
{
public:
    NotConnectedError()
        : std::runtime_error("Not connected to the Vicon Server.")
    {
    }
};

class BadResultError : public std::runtime_error
{
public:
    BadResultError(Result::Enum result) : std::runtime_error(to_string(result))
    {
    }
};

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

    friend std::ostream& operator<<(std::ostream& os, const ViconFrame& vf)
    {
        os << "Frame Number: " << vf.frame_number << std::endl;
        os << "Frame Rate: " << vf.frame_rate << std::endl;
        os << "Latency: " << vf.latency << std::endl;
        os << "Timestamp: " << vf.time_stamp << std::endl;

        os << "Subjects (" << vf.subjects.size() << "):" << std::endl;
        for (auto const& [subject_name, subject_data] : vf.subjects)
        {
            os << "  " << subject_name << std::endl;
            os << "    Visible: " << to_string(subject_data.is_visible)
               << std::endl;
            os << "    Translation: " << subject_data.global_translation
               << std::endl;
            os << "    Rotation: " << subject_data.global_rotation_quaternion
               << std::endl;
            os << "    Quality: " << subject_data.quality << std::endl;
        }

        return os;
    }
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
                  std::shared_ptr<spdlog::logger> logger = nullptr)
        : host_name_(host_name), config_(config)
    {
        if (logger)
        {
            log_ = logger;
        }
        else
        {
            log_ = spdlog::stderr_color_mt("ViconReceiver");
        }
    }

    ~ViconReceiver()
    {
        if (is_connected())
        {
            disconnect();
        }
    }

    bool is_connected() const
    {
        return client_.IsConnected().Connected;
    }

    void connect()
    {
        log_->info("Connecting to {}...", host_name_);
        while (!client_.IsConnected().Connected)
        {
            // Direct connection

            const Result::Enum connect_result =
                client_.Connect(host_name_).Result;
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
            if (client_.EnableLightweightSegmentData().Result !=
                Result::Success)
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

    void disconnect()
    {
        log_->info("Disconnecting...");
        client_.DisableSegmentData();
        client_.Disconnect();
    }

    void print_info() const
    {
        std::cout << "Version: " << client_.GetVersion() << std::endl;

        std::cout << "Segment Data Enabled: "
                  << to_string(client_.IsSegmentDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Lightweight Segment Data Enabled: "
                  << to_string(
                         client_.IsLightweightSegmentDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Marker Data Enabled: "
                  << to_string(client_.IsMarkerDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Unlabeled Marker Data Enabled: "
                  << to_string(client_.IsUnlabeledMarkerDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Device Data Enabled: "
                  << to_string(client_.IsDeviceDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Centroid Data Enabled: "
                  << to_string(client_.IsCentroidDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Marker Ray Data Enabled: "
                  << to_string(client_.IsMarkerRayDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Centroid Data Enabled: "
                  << to_string(client_.IsCentroidDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Greyscale Data Enabled: "
                  << to_string(client_.IsGreyscaleDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Video Data Enabled: "
                  << to_string(client_.IsVideoDataEnabled().Enabled)
                  << std::endl;
        std::cout << "Debug Data Enabled: "
                  << to_string(client_.IsDebugDataEnabled().Enabled)
                  << std::endl;

        Output_GetAxisMapping axis_mapping = client_.GetAxisMapping();
        std::cout << "Axis Mapping: X-" << axis_mapping.XAxis << " Y-"
                  << axis_mapping.YAxis << " Z-" << axis_mapping.ZAxis
                  << std::endl;
    }

    ViconFrame read()
    {
        ViconFrame frame;

        client_get_frame();

        // NOTE: This is only guaranteed to provide a UNIX timestamp
        // starting from C++20!
        // It would actually be better if the timestamp would be provided by
        // the Vicon system itself, but it doesn't seem to have this
        // functionality...
        frame.time_stamp =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
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
            Output_GetSegmentGlobalTranslation global_translation =
                client_.GetSegmentGlobalTranslation(subject_name, root_segment);
            Output_GetSegmentGlobalRotationQuaternion global_rotation =
                client_.GetSegmentGlobalRotationQuaternion(subject_name,
                                                           root_segment);

            subject_data.is_visible =
                !(global_translation.Occluded or global_rotation.Occluded);

            std::move(std::begin(global_translation.Translation),
                      std::end(global_translation.Translation),
                      subject_data.global_translation.begin());

            std::move(std::begin(global_rotation.Rotation),
                      std::end(global_rotation.Rotation),
                      subject_data.global_rotation_quaternion.begin());

            // Get the quality of the subject (object) if supported
            Output_GetObjectQuality quality =
                client_.GetObjectQuality(subject_name);
            subject_data.quality =
                (quality.Result == Result::Success) ? quality.Quality : 0.0;

            frame.subjects[subject_name] = subject_data;
        }

        return frame;
    }

    void print_latency_info() const
    {
        std::cout << "Latency: " << client_.GetLatencyTotal().Total << "s"
                  << std::endl;

        for (unsigned int LatencySampleIndex = 0;
             LatencySampleIndex < client_.GetLatencySampleCount().Count;
             ++LatencySampleIndex)
        {
            std::string SampleName =
                client_.GetLatencySampleName(LatencySampleIndex).Name;
            double SampleValue =
                client_.GetLatencySampleValue(SampleName).Value;

            std::cout << "  " << SampleName << " " << SampleValue << "s"
                      << std::endl;
        }
        std::cout << std::endl;
    }

    void filter_subjects(const std::vector<std::string> subjects)
    {
        // There needs to be a previously loaded frame in order to add subjects
        // to the filter.  Thus, check if there already is one and try to get
        // a new one if not.
        Result::Enum result = client_.GetFrameNumber().Result;
        switch (result)
        {
            case Result::Success:
                break;
            case Result::NoFrame:
                log_->info(
                    "Get initial frame before adding subjects to filter.");
                client_get_frame();
                break;
            default:
                throw BadResultError(result);
        }

        for (const std::string& subject_name : subjects)
        {
            log_->info("Add {} to subject filter", subject_name);
            Result::Enum result =
                client_.AddToSubjectFilter(subject_name).Result;
            if (result != Result::Success)
            {
                throw BadResultError(result);
            }
        }
    }

private:
    std::shared_ptr<spdlog::logger> log_;
    ViconDataStreamSDK::CPP::Client client_;

    const std::string host_name_;
    const ViconReceiverConfig config_;

    void client_get_frame()
    {
        Result::Enum result = client_.GetFrame().Result;

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
};

int main(int argc, char* argv[])
{
    auto logger = spdlog::get("root");
    if (!logger)
    {
        logger = spdlog::stderr_color_mt("root");
        // auto log_level = spdlog::level::from_str(config.logger_level);
        // log_->set_level(log_level);
    }

    // Program options

    int arg_num = 1;

    std::string host_name = "localhost:801";
    if (argc > 1 && strncmp(argv[arg_num], "--", 2) != 0)
    {
        host_name = argv[arg_num];
        arg_num++;
    }

    ViconReceiverConfig config;
    bool only_once = false;
    std::vector<std::string> filtered_subjects;

    for (int a = arg_num; a < argc; ++a)
    {
        std::string arg = argv[a];
        if (arg == "--help")
        {
            std::cout << argv[0]
                      << " <HostName>: allowed options include:" << std::endl;
            std::cout << " --help" << std::endl;
            std::cout << " --once" << std::endl;

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
        else
        {
            std::cerr << "Failed to understand argument <" << argv[a]
                      << ">...exiting" << std::endl;
            return 1;
        }
    }

    ViconReceiver receiver(host_name, config);
    receiver.connect();
    receiver.filter_subjects(filtered_subjects);
    receiver.print_info();

    std::cout << "\n==============================\n" << std::endl;

    //  Loop until a key is pressed
    while (true)
    {
        ViconFrame frame = receiver.read();
        receiver.print_latency_info();
        std::cout << frame << std::endl;

        if (only_once)
        {
            break;
        }
    }

    receiver.disconnect();
}
