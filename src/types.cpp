// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */
#include <vicon_transformer/types.hpp>

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

}  // namespace vicon_transformer
