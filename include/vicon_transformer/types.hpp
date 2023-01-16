/**
 * @file
 * @brief Data types used in the package
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <array>
#include <map>

#include <cereal/cereal.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

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
    bool is_visible;

    /**
     * @brief Position of the subject w.r.t. the global origin.
     *
     * This field is only set if @ref is_visible is true.
     */
    std::array<double, 3> global_translation;

    /**
     * @brief Orientation of the subject w.r.t. the global origin.
     *
     * The orientation is given as quaternion (x, y, z, w).
     *
     * This field is only set if @ref is_visible is true.
     */
    std::array<double, 4> global_rotation_quaternion;

    //! Quality measure of the pose estimation.
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
        int format_version = 3;
        archive(CEREAL_NVP(format_version));
        if (format_version != 3)
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

}  // namespace vicon_transformer
