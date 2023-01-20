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
/**
 * @brief Indicates that the client is not connected to a Vicon server.
 */
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

/**
 * @brief The requested subject does not exist
 *
 * The subject name is provided in the exception's message.
 */
class UnknownSubjectError : public std::runtime_error
{
public:
    UnknownSubjectError(const std::string& subject_name)
        : std::runtime_error(fmt::format("{}", subject_name))
    {
    }
};

/**
 * @brief The subject is not visible (and thus no transform data available).
 *
 * The actual result is provided in the exception's message.
 */
class SubjectNotVisibleError : public std::runtime_error
{
public:
    SubjectNotVisibleError(const std::string& subject_name)
        : std::runtime_error(fmt::format("{}", subject_name))
    {
    }
};

}  // namespace vicon_transformer
