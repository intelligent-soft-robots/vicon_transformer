/**
 * @file
 * @brief Tests for vicon_transformer.hpp
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#ifndef TEST_DATA_FILE_DIR
#error TEST_DATA_FILE_DIR is not defined
#endif

#include <memory>

#include <gtest/gtest.h>

#include <vicon_transformer/vicon_receiver.hpp>
#include <vicon_transformer/vicon_transformer.hpp>

#include "utils.hpp"

using vicon_transformer::JsonReceiver;
using vicon_transformer::Transformation;
using vicon_transformer::ViconFrame;
using vicon_transformer::ViconTransformer;

namespace
{
std::shared_ptr<JsonReceiver> get_receiver()
{
    std::string file =
        std::string(TEST_DATA_FILE_DIR) + "/test_frame1_format3.json";
    std::shared_ptr<JsonReceiver> receiver =
        std::make_shared<JsonReceiver>(file);

    return receiver;
}
}  // namespace

TEST(ViconTransformer, get_timestamp)
{
    // leave origin subject name empty, so not origin transform is done
    ViconTransformer vtf(get_receiver(), "");
    vtf.update();

    ASSERT_EQ(vtf.get_timestamp_ns(), 1638538681615901200);
}

TEST(ViconTransformer, set_frame)
{
    // leave origin subject name empty, so not origin transform is done
    ViconTransformer vtf(get_receiver(), "");
    vtf.update();

    // get the original frame from the receiver, modify it and set it back
    ViconFrame frame = vtf.receiver()->read();
    frame.time_stamp = 42;
    vtf.set_frame(frame);

    ASSERT_EQ(vtf.get_timestamp_ns(), 42);
}

TEST(ViconTransformer, get_raw_transforms)
{
    // leave origin subject name empty, so not origin transform is done
    ViconTransformer vtf(get_receiver(), "");
    vtf.update();

    Transformation tf_raw = vtf.get_raw_transform("Marker Ballmaschine");
    Transformation tf = vtf.get_transform("Marker Ballmaschine");

    // verify both transforms are equal
    ASSERT_MATRIX_ALMOST_EQUAL(tf_raw.matrix(), tf.matrix());

    ASSERT_QUATERNION_ALMOST_EQUAL(tf.rotation,
                                   Eigen::Quaterniond(0.8840510643008075,
                                                      0.0027011836358849634,
                                                      0.010576271408817553,
                                                      0.46726284016457137));
    ASSERT_MATRIX_ALMOST_EQUAL(
        tf.translation,
        Eigen::Vector3d(
            -2.4118746572218347, .11181820474947762, .5197499656025113));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
