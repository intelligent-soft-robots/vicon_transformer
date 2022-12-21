/**
 * @file
 * @brief Tests for vicon_transformer.hpp
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <memory>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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
    // assumes test is executed in package root directory
    std::string file = "tests/data/test_frame1_format3.json";

    std::shared_ptr<JsonReceiver> receiver =
        std::make_shared<JsonReceiver>(file);

    return receiver;
}
}  // namespace

TEST(ViconTransformer, get_timestamp)
{
    ViconTransformer vtf(get_receiver(), "");
    vtf.update();

    ASSERT_EQ(vtf.get_timestamp_ns(), 1638538681615901200);
}

TEST(ViconTransformer, set_frame)
{
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

TEST(ViconTransformer, get_subject_names)
{
    ViconTransformer vtf(get_receiver(), "");
    vtf.update();

    auto names = vtf.get_subject_names();
    std::vector<std::string> expected = {"Marker Ballmaschine",
                                         "Marker_Arm",
                                         "rll_led_stick",
                                         "rll_muscle_base",
                                         "rll_muscle_racket",
                                         "rll_ping_base",
                                         "TT Platte_Eckteil 1",
                                         "TT Platte_Eckteil 2",
                                         "TT Platte_Eckteil 3",
                                         "TT Platte_Eckteil 4"};

    ASSERT_THAT(names, testing::UnorderedElementsAreArray(expected));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
