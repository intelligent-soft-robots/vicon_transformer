// SPDX-License-Identifier: BSD-3-Clause

/**
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <cereal/archives/binary.hpp>
#include <vicon_transformer/vicon_transformer.hpp>

#include <spdlog/spdlog.h>

#include <vicon_transformer/errors.hpp>

namespace vicon_transformer
{
ViconTransformer::ViconTransformer(std::shared_ptr<Receiver> receiver,
                                   const std::string &origin_subject_name,
                                   std::shared_ptr<spdlog::logger> logger)
    : receiver_(receiver),
      origin_subject_name_(origin_subject_name),
      origin_tf_(Transformation::Identity())
{
    if (logger)
    {
        log_ = logger;
    }
    else
    {
        const std::string name = "ViconTransformer";
        if (!(log_ = spdlog::get(name)))
        {
            log_ = spdlog::stderr_color_mt(name);
            log_->set_level(spdlog::level::debug);
        }
    }
}

std::shared_ptr<Receiver> ViconTransformer::receiver()
{
    return receiver_;
}

std::shared_ptr<const Receiver> ViconTransformer::receiver() const
{
    return receiver_;
}

void ViconTransformer::update()
{
    set_frame(receiver_->read());
}

void ViconTransformer::set_frame(const ViconFrame &frame)
{
    frame_ = frame;

    // TODO: should this be updated for every frame or only once in the
    // beginning?
    if (!origin_subject_name_.empty())
    {
        origin_tf_ = get_raw_transform(origin_subject_name_).inverse();
    }
}

void ViconTransformer::wait_for_origin_subject_data()
{
    // nothing to wait for if no origin subject is set
    if (origin_subject_name_.empty())
    {
        log_->debug("Do not wait for origin pose as no origin subject is set.");
        return;
    }

    log_->info("Wait for valid origin subject pose...");
    while (true)
    {
        log_->debug("get new frame");
        try
        {
            update();
            log_->info("Got origin subject pose.");
            // break loop if update was successful
            break;
        }
        catch (const SubjectNotVisibleError &)
        {
            // do nothing
        }
    }
}

int64_t ViconTransformer::get_timestamp_ns() const
{
    return frame_.time_stamp;
}

std::vector<std::string> ViconTransformer::get_subject_names() const
{
    std::vector<std::string> names;
    names.reserve(frame_.subjects.size());
    for (const auto &[name, _] : frame_.subjects)
    {
        names.push_back(name);
    }
    return names;
}

bool ViconTransformer::is_visible(const std::string &subject_name) const
{
    try
    {
        return get_subject_data(subject_name).is_visible;
    }
    catch (const std::out_of_range &)
    {
        throw UnknownSubjectError(subject_name);
    }
}

Transformation ViconTransformer::get_transform(
    const std::string &subject_name) const
{
    auto tf = get_raw_transform(subject_name);
    if (is_visible(subject_name))
    {
        return origin_tf_ * tf;
    }
    else
    {
        // If subject is not visible, its pose should be set to identity.
        // Applying the origin transform would result in something that looks
        // like a legit pose while it is actually just garbage.  Thus simply
        // forward the identity pose instead, which makes it more obvious that
        // the subject is not actually visible when looking at the data.
        return tf;
    }
}

Transformation ViconTransformer::get_raw_transform(
    const std::string &subject_name) const
{
    SubjectData sd = get_subject_data(subject_name);

    if (!sd.is_visible)
    {
        throw SubjectNotVisibleError(subject_name);
    }

    return sd.global_pose;
}

const SubjectData &ViconTransformer::get_subject_data(
    const std::string &subject_name) const
{
    try
    {
        return frame_.subjects.at(subject_name);
    }
    catch (const std::out_of_range &)
    {
        throw UnknownSubjectError(subject_name);
    }
}

ViconFrame ViconTransformer::get_frame() const
{
    ViconFrame transformed_frame = frame_;

    for (auto &[name, data] : transformed_frame.subjects)
    {
        // only apply origin transform if the subject is actually visible
        if (data.is_visible)
        {
            data.global_pose = origin_tf_ * data.global_pose;
        }
    }

    return transformed_frame;
}

}  // namespace vicon_transformer
