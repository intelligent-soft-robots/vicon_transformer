/**
 * @file
 * @brief ostream operator overloads for various types.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <iterator>

#include <vicon-datastream-sdk/DataStreamClient.h>

namespace ViconDataStreamSDK::CPP
{
inline std::ostream& operator<<(
    std::ostream& os, const ViconDataStreamSDK::CPP::Output_GetVersion& x)
{
    os << x.Major << "." << x.Minor << "." << x.Point << "." << x.Revision;
    return os;
}

inline std::ostream& operator<<(
    std::ostream& os, const ViconDataStreamSDK::CPP::Direction::Enum& dir)
{
    using Direction = ViconDataStreamSDK::CPP::Direction::Enum;
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

inline std::ostream& operator<<(
    std::ostream& os, const ViconDataStreamSDK::CPP::Result::Enum& result)
{
    using Result = ViconDataStreamSDK::CPP::Result::Enum;
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

}  // namespace vicon_transformer
