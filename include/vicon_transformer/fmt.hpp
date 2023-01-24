// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief fmt::formatter implementations for various types.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <fmt/format.h>
#include <vicon-datastream-sdk/DataStreamClient.h>

template <>
struct fmt::formatter<ViconDataStreamSDK::CPP::Output_GetVersion>
    : formatter<string_view>
{
    template <typename FormatContext>
    auto format(const ViconDataStreamSDK::CPP::Output_GetVersion& version,
                FormatContext& ctx)
    {
        string_view str = fmt::format("{}.{}.{}.{}",
                                      version.Major,
                                      version.Minor,
                                      version.Point,
                                      version.Revision);
        return formatter<string_view>::format(str, ctx);
    }
};

template <>
struct fmt::formatter<ViconDataStreamSDK::CPP::Direction::Enum>
    : formatter<string_view>
{
    template <typename FormatContext>
    auto format(const ViconDataStreamSDK::CPP::Direction::Enum& dir,
                FormatContext& ctx)
    {
        using Direction = ViconDataStreamSDK::CPP::Direction::Enum;

        string_view name = "???";
        switch (dir)
        {
            case Direction::Forward:
                name = "Forward";
                break;
            case Direction::Backward:
                name = "Backward";
                break;
            case Direction::Left:
                name = "Left";
                break;
            case Direction::Right:
                name = "Right";
                break;
            case Direction::Up:
                name = "Up";
                break;
            case Direction::Down:
                name = "Down";
                break;
            default:
                name = "Unknown";
                break;
        }

        return formatter<string_view>::format(name, ctx);
    }
};

template <>
struct fmt::formatter<ViconDataStreamSDK::CPP::Result::Enum>
    : formatter<string_view>
{
    template <typename FormatContext>
    auto format(const ViconDataStreamSDK::CPP::Result::Enum& result,
                FormatContext& ctx)
    {
        using Result = ViconDataStreamSDK::CPP::Result::Enum;

        string_view name = "???";
        switch (result)
        {
            case Result::Unknown:
                name = "Unknown";
                break;
            case Result::NotImplemented:
                name = "NotImplemented";
                break;
            case Result::Success:
                name = "Success";
                break;
            case Result::InvalidHostName:
                name = "InvalidHostName";
                break;
            case Result::InvalidMulticastIP:
                name = "InvalidMulticastIP";
                break;
            case Result::ClientAlreadyConnected:
                name = "ClientAlreadyConnected";
                break;
            case Result::ClientConnectionFailed:
                name = "ClientConnectionFailed";
                break;
            case Result::ServerAlreadyTransmittingMulticast:
                name = "ServerAlreadyTransmittingMulticast";
                break;
            case Result::ServerNotTransmittingMulticast:
                name = "ServerNotTransmittingMulticast";
                break;
            case Result::NotConnected:
                name = "NotConnected";
                break;
            case Result::NoFrame:
                name = "NoFrame";
                break;
            case Result::InvalidIndex:
                name = "InvalidIndex";
                break;
            case Result::InvalidCameraName:
                name = "InvalidCameraName";
                break;
            case Result::InvalidSubjectName:
                name = "InvalidSubjectName";
                break;
            case Result::InvalidSegmentName:
                name = "InvalidSegmentName";
                break;
            case Result::InvalidMarkerName:
                name = "InvalidMarkerName";
                break;
            case Result::InvalidDeviceName:
                name = "InvalidDeviceName";
                break;
            case Result::InvalidDeviceOutputName:
                name = "InvalidDeviceOutputName";
                break;
            case Result::InvalidLatencySampleName:
                name = "InvalidLatencySampleName";
                break;
            case Result::CoLinearAxes:
                name = "CoLinearAxes";
                break;
            case Result::LeftHandedAxes:
                name = "LeftHandedAxes";
                break;
            case Result::HapticAlreadySet:
                name = "HapticAlreadySet";
                break;
            case Result::EarlyDataRequested:
                name = "EarlyDataRequested";
                break;
            case Result::LateDataRequested:
                name = "LateDataRequested";
                break;
            case Result::InvalidOperation:
                name = "InvalidOperation";
                break;
            case Result::NotSupported:
                name = "NotSupported";
                break;
            case Result::ConfigurationFailed:
                name = "ConfigurationFailed";
                break;
            case Result::NotPresent:
                name = "NotPresent";
                break;
        }
        return formatter<string_view>::format(name, ctx);
    }
};
