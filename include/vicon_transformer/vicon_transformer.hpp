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
class ViconTransformerBase  // TODO better name
{
public:
    ViconTransformerBase(const std::string &origin_subject_name,
                         std::shared_ptr<spdlog::logger> logger = nullptr);

    void set_frame(const ViconFrame &frame);

    int64_t get_timestamp_ns() const;

    std::vector<std::string> get_subject_names() const;

    bool is_visible(const std::string &subject_name) const;

    Transformation get_transform(const std::string &subject_name) const;

    // private?
    Transformation get_raw_transform(const std::string &subject_name) const;

protected:
    std::shared_ptr<spdlog::logger> log_;
    std::string origin_subject_name_;
    ViconFrame frame_;
    Transformation origin_tf_;
};

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

    std::shared_ptr<Receiver> receiver()
    {
        return receiver_;
    }
    std::shared_ptr<const Receiver> receiver() const
    {
        return receiver_;
    }

    void update()
    {
        set_frame(receiver_->read());
    }

private:
    std::shared_ptr<Receiver> receiver_;
};

}  // namespace vicon_transformer
