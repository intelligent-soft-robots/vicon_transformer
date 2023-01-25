// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief PAM-Vicon specific settings for the o80 driver.
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <string>

#include "o80_driver.hpp"
#include "o80_standalone.hpp"

// TODO move to separate package (already using different namespace)
namespace pam_vicon_o80
{
//! Number of subjects in the PAM Vicon setup.
constexpr std::size_t NUM_SUBJECTS = 10;

/**
 * @brief Enumeration of Vicon subjects.
 *
 * The values of the enum entries are used as indices for arrays with subject
 * data.
 */
enum Subjects
{
    PING_BASE = 0,
    BALL_LAUNCHER,
    ARM,
    TABLE_CORNER_1,
    TABLE_CORNER_2,
    TABLE_CORNER_3,
    TABLE_CORNER_4,
    LED_STICK,
    MUSCLE_BASE,
    MUSCLE_RACKET
};

//! Maps subject name to @ref Subjects entry.
const std::map<std::string, size_t> _subject_name_to_index = {
    {"rll_ping_base", PING_BASE},
    {"Marker Ballmaschine", BALL_LAUNCHER},
    {"Marker_Arm", ARM},
    {"TT Platte_Eckteil 1", TABLE_CORNER_1},
    {"TT Platte_Eckteil 2", TABLE_CORNER_2},
    {"TT Platte_Eckteil 3", TABLE_CORNER_3},
    {"TT Platte_Eckteil 4", TABLE_CORNER_4},
    {"rll_led_stick", LED_STICK},
    {"rll_muscle_base", MUSCLE_BASE},
    {"rll_muscle_racket", MUSCLE_RACKET},
};

/**
 * @brief Maps subject name to index.
 *
 * @param name Name of the subject.
 *
 * @return Index at which this subject is expected in subject arrays.
 */
size_t map_subject_name_to_index(const std::string &name)
{
    return _subject_name_to_index.at(name);
}

/**
 * @brief Get list of subject names.
 *
 * The names are ordered according to @ref Subjects, i.e.
 *
 * .. code-block:: C++
 *
 *    auto names = get_subject_names();
 *    names[Subjects::BALL_LAUNCHER]
 *
 * gives the name of the ball launcher marker.
 *
 * @return Array that maps subject index to its name.
 */
std::array<std::string, NUM_SUBJECTS> get_subject_names()
{
    std::array<std::string, NUM_SUBJECTS> names;

    for (auto &[name, index] : _subject_name_to_index)
    {
        names[index] = name;
    }

    return names;
}

//! FixedSizeViconFrame for the PAM Vicon setup.
typedef vicon_transformer::FixedSizeViconFrame<NUM_SUBJECTS>
    FixedSizeViconFrame;
//! o80Driver for the PAM Vicon setup
typedef vicon_transformer::o80Driver<NUM_SUBJECTS, map_subject_name_to_index>
    o80Driver;
//! o80Standalone for the PAM Vicon setup
typedef vicon_transformer::o80Standalone<o80Driver> o80Standalone;

}  // namespace pam_vicon_o80
