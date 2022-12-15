/**
 * @file
 * @brief Custom exceptions.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <fmt/format.h>
#include <vicon-datastream-sdk/DataStreamClient.h>

#include <vicon_transformer/fmt.hpp>

namespace vicon_transformer
{
class NotConnectedError : public std::runtime_error
{
public:
    NotConnectedError()
        : std::runtime_error("Not connected to the Vicon Server.")
    {
    }
};

/**
 * @brief Indicates that a Vicon SDK function returned a bad result.
 *
 * The actual result is provided in the exception's message.
 */
class BadResultError : public std::runtime_error
{
public:
    BadResultError(ViconDataStreamSDK::CPP::Result::Enum result)
        : std::runtime_error(fmt::format("{}", result))
    {
    }
};

}  // namespace vicon_transformer
