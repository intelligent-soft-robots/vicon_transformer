// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file
 * @brief o80 driver for the Vicon system
 * @copyright 2023 Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <map>
#include <memory>
#include <string>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <o80/driver.hpp>

#include "vicon_transformer.hpp"

namespace vicon_transformer
{
class None
{
};

/**
 * @brief Generic o80 driver to provide Vicon data.
 *
 * Since the shared memory used by o80 requires observations to be of fixed
 * size, the number of subjects observed by Vicon needs to be provided at
 * compile time via the template argument.
 *
 * Likewise the subject names are not stored in the observation.  Instead the
 * subject data is provided in an array with an order which is defined by the
 * ``map_name_to_index`` function.  The client code on the other end will
 * likewise need to have access to this mapping to know which subject is listed
 * at which position.
 *
 * ``map_name_to_index`` is expected to throw an @ref UnknownSubjectError if an
 * unexpected name is passed to it.  In this case, the driver will ignore that
 * subject.
 *
 * Subjects for which no information is provided by Vicon will have the
 * ``is_visible`` field set to false.
 *
 * The driver uses @ref ViconTransformer with the given receiver to acquire the
 * Vicon frames and provide poses relative to the specified origin subject.
 *
 * @tparam NUM_SUBJECTS Number of subjects.
 * @tparam map_name_to_index Function that maps a subject name to an index in
 *      the subject array.  The indices must be less than ``NUM_SUBJECTS - 1``.
 */
template <size_t NUM_SUBJECTS, size_t (*map_name_to_index)(const std::string&)>
class o80Driver : public o80::Driver<None, FixedSizeViconFrame<NUM_SUBJECTS>>
{
public:
    /**
     * @param receiver Initialised receiver instance which provides Vicon frames
     * @param origin_subject_name Name of the origin subject.  Has to be one of
     *      the subjects that is tracked by Vicon.  Poses of all subjects will
     *      be given relative to the origin subject.
     * @param logger A logger instance used for logging output.
     */
    o80Driver(std::shared_ptr<vicon_transformer::Receiver> receiver,
              const std::string& origin_subject_name,
              std::shared_ptr<spdlog::logger> logger = nullptr)
        : vicon_transformer_(receiver, origin_subject_name, logger)
    {
        if (logger)
        {
            log_ = logger;
        }
        else
        {
            const std::string name = "Vicon o80 Driver";
            if (!(log_ = spdlog::get(name)))
            {
                log_ = spdlog::stderr_color_mt(name);
                log_->set_level(spdlog::level::debug);
            }
        }
    }

    void start() override
    {
        vicon_transformer_.wait_for_origin_subject_data();
    }

    void stop() override
    {
        // do nothing
    }

    void set(const None&) override
    {
        // do nothing
    }

    FixedSizeViconFrame<NUM_SUBJECTS> get() override
    {
        // get frame from vicon_transformer_ and convert it to a fixed-size
        // frame, following the mapping provided by map_name_to_index().

        typedef FixedSizeViconFrame<NUM_SUBJECTS> _Fixed;

        vicon_transformer_.update();
        ViconFrame frame = vicon_transformer_.get_frame();

        _Fixed fixed_frame;
        fixed_frame.frame_number = frame.frame_number;
        fixed_frame.frame_rate = frame.frame_rate;
        fixed_frame.latency = frame.latency;
        fixed_frame.time_stamp = frame.time_stamp;

        for (auto& [name, data] : frame.subjects)
        {
            size_t i;
            try
            {
                i = map_name_to_index(name);
            }
            catch (const UnknownSubjectError&)
            {
                // Ignore unexpected subjects but print a warning the first time
                // they occur.

                static std::map<std::string, bool> already_warned;

                // bools are value-initialized to false, so if 'name' is not yet
                // in the map, a new entry will created with the value false.
                if (!already_warned[name])
                {
                    log_->warn("Ignore unexpected subject '{}'", name);
                    already_warned[name] = true;
                }

                continue;
            }

            if (i >= NUM_SUBJECTS)
            {
                throw std::out_of_range(
                    fmt::format("Subject '{}' is mapped to index {} which "
                                "exceeds capacity of FixedSizeViconFrame<{}>.",
                                name,
                                i,
                                NUM_SUBJECTS));
            }

            fixed_frame.subjects[i] = data;
        }

        return fixed_frame;
    }

private:
    ViconTransformer vicon_transformer_;
    std::shared_ptr<spdlog::logger> log_;
};

}  // namespace vicon_transformer
