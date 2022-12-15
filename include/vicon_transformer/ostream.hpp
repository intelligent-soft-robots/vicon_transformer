/**
 * @file
 * @brief ostream operator overloads for various types.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <iterator>

#include <vicon-datastream-sdk/DataStreamClient.h>

namespace vicon_transformer
{
namespace VDSSDK = ViconDataStreamSDK::CPP;

template <class T, std::size_t N>
std::ostream& operator<<(std::ostream& os, const std::array<T, N>& arr);

std::ostream& operator<<(std::ostream& os, const VDSSDK::Output_GetVersion& x);

std::ostream& operator<<(std::ostream& os, const VDSSDK::Direction::Enum& dir);

std::ostream& operator<<(std::ostream& os, const VDSSDK::Result::Enum& result);

}  // namespace vicon_transformer
