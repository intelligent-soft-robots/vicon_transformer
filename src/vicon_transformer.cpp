/**
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
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
    return origin_tf_ * get_raw_transform(subject_name);
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

}  // namespace vicon_transformer
