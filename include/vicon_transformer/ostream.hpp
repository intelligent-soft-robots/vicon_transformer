/**
 * @file
 * @brief ostream operator overloads for various types.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <fmt/ostream.h>
#include <iterator>

#include <vicon-datastream-sdk/DataStreamClient.h>

namespace ViconDataStreamSDK::CPP
{
inline std::ostream& operator<<(
    std::ostream& os, const ViconDataStreamSDK::CPP::Output_GetVersion& x)
{
    fmt::print(os, "{}", x);
    return os;
}

inline std::ostream& operator<<(
    std::ostream& os, const ViconDataStreamSDK::CPP::Direction::Enum& dir)
{
    fmt::print(os, "{}", dir);
    return os;
}

inline std::ostream& operator<<(
    std::ostream& os, const ViconDataStreamSDK::CPP::Result::Enum& result)
{
    fmt::print(os, "{}", result);
    return os;
}

}  // namespace ViconDataStreamSDK::CPP
