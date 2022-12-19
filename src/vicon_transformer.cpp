/**
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <vicon_transformer/vicon_transformer.hpp>

namespace vicon_transformer
{
ViconTransformerBase::ViconTransformerBase(
    const std::string &origin_subject_name,
    std::shared_ptr<spdlog::logger> logger)
    : origin_subject_name_(origin_subject_name),
      origin_tf_(Transformation::Identity())
{
    if (logger)
    {
        log_ = logger;
    }
    else
    {
        log_ = spdlog::stderr_color_mt("ViconTransformer");
    }
}

void ViconTransformerBase::set_frame(const ViconFrame &frame)
{
    frame_ = frame;

    // TODO: should this be updated for every frame or only once in the
    // beginning?
    if (!origin_subject_name_.empty())
    {
        origin_tf_ = get_raw_transform(origin_subject_name_);
    }
}

int64_t ViconTransformerBase::get_timestamp_ns() const
{
    return frame_.time_stamp;
}

std::vector<std::string> ViconTransformerBase::get_subject_names() const
{
    std::vector<std::string> names;
    names.reserve(frame_.subjects.size());
    for (const auto &[name, _] : frame_.subjects)
    {
        names.push_back(name);
    }
    return names;
}

bool ViconTransformerBase::is_visible(const std::string &subject_name) const
{
    return frame_.subjects.at(subject_name).is_visible;
}

Transformation ViconTransformerBase::get_transform(
    const std::string &subject_name) const
{
    return origin_tf_ * get_raw_transform(subject_name);
}

Transformation ViconTransformerBase::get_raw_transform(
    const std::string &subject_name) const
{
    SubjectData sd = frame_.subjects.at(subject_name);

    // NOTE: SubjectData provides quaternion in (x, y, z, w) format but Eigen
    // expected (w, x, y, z).
    const auto &[qx, qy, qz, qw] = sd.global_rotation_quaternion;
    Eigen::Quaterniond rotation(qw, qx, qy, qz);

    // NOTE: Vicon provides translation in millimetres, so needs to be converted
    // to metres
    Eigen::Vector3d translation =
        Eigen::Map<Eigen::Vector3d>(sd.global_translation.data()) / 1000;

    return Transformation(rotation, translation);
}

}  // namespace vicon_transformer
