// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief Data types used in the package
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <array>
#include <map>

#include <fmt/ostream.h>
#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

#include "transform.hpp"

namespace vicon_transformer
{
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
    bool is_visible = false;

    /**
     * @brief Pose of the subject w.r.t. the global origin.
     *
     * This field is only set if @ref is_visible is true.
     */
    Transformation global_pose;

    //! Quality measure of the pose estimation.
    double quality = 0.0;

    template <class Archive>
    void serialize(Archive& archive)
    {
        archive(CEREAL_NVP(is_visible),
                CEREAL_NVP(global_pose),
                CEREAL_NVP(quality));
    }
};

// Format 3 version of SubjectData.  Only here for backwards compatibility with
// old recordings, do not use this in any new code!
struct _SubjectData_v3
{
    bool is_visible;
    std::array<double, 3> global_translation;
    std::array<double, 4> global_rotation_quaternion;
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
        constexpr int LATEST_FORMAT = 4;

        int format_version = LATEST_FORMAT;
        archive(CEREAL_NVP(format_version));

        if (format_version == 3)
        {
            // This can only happen if we are loading, otherwise, format_version
            // should have the value of LATEST_FORMAT, which is != 3.

            // In format 3, the ViconFrame itself was the same but SubjectData
            // was different
            std::map<std::string, _SubjectData_v3> subjects_v3;
            archive(CEREAL_NVP(frame_number),
                    CEREAL_NVP(frame_rate),
                    CEREAL_NVP(latency),
                    CEREAL_NVP(time_stamp),
                    CEREAL_NVP(subjects_v3));

            subjects.clear();
            for (auto const& [key, val] : subjects_v3)
            {
                SubjectData sd;
                sd.is_visible = val.is_visible;
                sd.quality = val.quality;

                sd.global_pose.translation.x() =
                    val.global_translation[0] / 1000;
                sd.global_pose.translation.y() =
                    val.global_translation[1] / 1000;
                sd.global_pose.translation.z() =
                    val.global_translation[2] / 1000;
                sd.global_pose.rotation.x() = val.global_rotation_quaternion[0];
                sd.global_pose.rotation.y() = val.global_rotation_quaternion[1];
                sd.global_pose.rotation.z() = val.global_rotation_quaternion[2];
                sd.global_pose.rotation.w() = val.global_rotation_quaternion[3];

                subjects[key] = sd;
            }

            return;
        }

        if (format_version != LATEST_FORMAT)
        {
            throw std::runtime_error(
                fmt::format("Invalid input format.  Expected format version {} "
                            "but archive has {}",
                            LATEST_FORMAT,
                            format_version));
        }

        archive(CEREAL_NVP(frame_number),
                CEREAL_NVP(frame_rate),
                CEREAL_NVP(latency),
                CEREAL_NVP(time_stamp),
                CEREAL_NVP(subjects));
    }
};

/**
 * @brief This is an alternative to ViconFrame with a fixed number of subjects.
 *
 * For some applications like o80 the data structure needs to be of fixed size.
 * This is not the case in @ref ViconFrame due to the use of std::map for the
 * subjects.  FixedSizeViconFrame can be used as an (less flexible) alternative
 * for these applications.
 *
 * Note that here, the names of the subjects are not stored, so one needs to
 * keep track of the order of subjects in a different way (e.g. by having a
 * fixed mapping from subject name to index).
 */
template <size_t NUM_SUBJECTS>
struct FixedSizeViconFrame
{
    static constexpr size_t max_num_subjects = NUM_SUBJECTS;

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
    std::array<SubjectData, NUM_SUBJECTS> subjects;

    template <size_t N>
    friend std::ostream& operator<<(std::ostream& os,
                                    const FixedSizeViconFrame<N>& vf);

    template <class Archive>
    void serialize(Archive& archive)
    {
        int format_version = 4;
        archive(CEREAL_NVP(format_version));
        if (format_version != 4)
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

template <size_t N>
std::ostream& operator<<(std::ostream& os, const FixedSizeViconFrame<N>& vf)
{
    fmt::print(os, "Frame Number: {}\n", vf.frame_number);
    fmt::print(os, "Frame Rate: {}\n", vf.frame_rate);
    fmt::print(os, "Latency: {}\n", vf.latency);
    fmt::print(os, "Timestamp: {}\n", vf.time_stamp);

    fmt::print(os, "Subjects ({}):\n", vf.subjects.size());
    for (auto const& data : vf.subjects)
    {
        fmt::print(os, "    ---\n");
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
