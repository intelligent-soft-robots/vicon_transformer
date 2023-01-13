/**
 * @file
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "transform.hpp"
#include "vicon_receiver.hpp"

namespace vicon_transformer
{
/**
 * @brief Base class for ViconTransformer
 *
 * This base class includes all methods that do not depend on the template
 * argument of ViconTransformer.  This allows to implement them in the cpp
 * file.
 */
class ViconTransformerBase  // TODO better name
{
public:
    /**
     * @param origin_subject_name Name of the subject that shall be used as
     * origin.
     * @param logger A logger instance used for logging output.  If not set, a
     *      logger with name "ViconTransformer" used.
     */
    ViconTransformerBase(const std::string &origin_subject_name,
                         std::shared_ptr<spdlog::logger> logger = nullptr);

    //! Set the Vicon frame that is used by the transformer.
    void set_frame(const ViconFrame &frame);

    //! Get timestamp of the frame in nanoseconds.
    int64_t get_timestamp_ns() const;

    //! Get list with the names of all registered subjects.
    std::vector<std::string> get_subject_names() const;

    /**
     * @brief Check if the specified subject is visible.
     *
     * @param subject_name Name of the subject.
     * @return True if it is visible in the current frame, false if not
     * @throws UnknownSubjectError if there is no subject with the given name.
     */
    bool is_visible(const std::string &subject_name) const;

    /**
     * @brief Get transformation of a subject relative to the origin subject.
     *
     * @param subject_name  Name of the subject
     * @return Transformation from the origin subject to the requested subject.
     * @throws SubjectNotVisibleError if the subject is not visible in the
     *      current frame.
     */
    Transformation get_transform(const std::string &subject_name) const;

    /**
     * @brief Get transformation of a subject relative to Vicons global origin.
     *
     * @note The Vicon origin can be at an arbitrary pose and can vary over
     * time if the system is recalibrated.  For more reliable results, specify a
     * static object as "origin subject" and use @ref get_transform instead of
     * this method.
     *
     * @param subject_name  Name of the subject
     * @return Transformation from the Vicon origin to the requested subject.
     * @throws SubjectNotVisibleError if the subject is not visible in the
     *      current frame.
     */
    Transformation get_raw_transform(const std::string &subject_name) const;

protected:
    std::shared_ptr<spdlog::logger> log_;
    std::string origin_subject_name_;
    ViconFrame frame_;
    Transformation origin_tf_;

    const SubjectData &get_subject_data(const std::string &subject_name) const;
};

/**
 * @brief Get data from a ViconReceiver and provide poses of subjects relative
 * to an "origin subject".
 *
 * Vicon provides positions and orientations of subjects relative to an origin
 * that is defined when calibrating the system.  Unfortunately, it is not easy
 * precisely specify this, so the actual origin can be in a not very well
 * defined location and can vary over time if the system is recalibrated.
 *
 * Instead, this class provides the option to specify a static object which
 * doesn't move over time (e.g. some markers attached to a wall) as "origin
 * subject" and provide poses of all other subjects relative to this origin
 * subject.  This makes the poses independent of the actual origin used by Vicon
 * and will give repeatable results, even if the system is recalibrated in
 * between (at least as long as the markers of the origin subject are not
 * moved).
 *
 * @tparam Receiver The receiver class that is used.  Use ViconReceiver when
 *      working with an actual Vicon system.
 */
template <typename Receiver>
class ViconTransformer : public ViconTransformerBase
{
public:
    ViconTransformer(std::shared_ptr<Receiver> receiver,
                     const std::string &origin_subject_name,
                     std::shared_ptr<spdlog::logger> logger = nullptr)
        : ViconTransformerBase(origin_subject_name, logger), receiver_(receiver)
    {
    }

    //! Return a pointer to the receiver instance.
    std::shared_ptr<Receiver> receiver()
    {
        return receiver_;
    }
    //! Return a const pointer to the receiver instance.
    std::shared_ptr<const Receiver> receiver() const
    {
        return receiver_;
    }

    //! Update transformations by getting a new frame from the receiver.
    void update()
    {
        set_frame(receiver_->read());
    }

private:
    std::shared_ptr<Receiver> receiver_;
};

}  // namespace vicon_transformer
