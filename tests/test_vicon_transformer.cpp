/**
 * @file
 * @brief Tests for vicon_transformer.hpp
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <memory>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vicon_transformer/errors.hpp>
#include <vicon_transformer/vicon_receiver.hpp>
#include <vicon_transformer/vicon_transformer.hpp>

#include "utils.hpp"

using vicon_transformer::JsonReceiver;
using vicon_transformer::Transformation;
using vicon_transformer::ViconFrame;
using vicon_transformer::ViconTransformer;

namespace
{
std::shared_ptr<JsonReceiver> get_receiver(const std::string &test_file)
{
    // assumes test is executed in package root directory
    std::string file = "tests/data/" + test_file;

    std::shared_ptr<JsonReceiver> receiver =
        std::make_shared<JsonReceiver>(file);

    return receiver;
}
}  // namespace

TEST(ViconTransformer, get_timestamp)
{
    ViconTransformer vtf(get_receiver("test_frame1.json"), "");
    vtf.update();

    ASSERT_EQ(vtf.get_timestamp_ns(), 1638538681615901200);
}

TEST(ViconTransformer, set_frame)
{
    ViconTransformer vtf(get_receiver("test_frame1.json"), "");
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
    ViconTransformer vtf(get_receiver("test_frame1.json"), "");
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
    ViconTransformer vtf(get_receiver("test_frame1.json"), "");
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

TEST(ViconTransformer, is_visible)
{
    ViconTransformer vtf(get_receiver("frame_with_missing_subjects.json"), "");
    vtf.update();

    EXPECT_TRUE(vtf.is_visible("Marker Ballmaschine"));
    EXPECT_TRUE(vtf.is_visible("Marker_Arm"));
    EXPECT_FALSE(vtf.is_visible("rll_led_stick"));
    EXPECT_FALSE(vtf.is_visible("rll_muscle_racket"));
}

TEST(ViconTransformer, unknown_subject_error)
{
    ViconTransformer vtf(get_receiver("frame_with_missing_subjects.json"), "");
    vtf.update();

    EXPECT_THROW(vtf.is_visible("foo"), vicon_transformer::UnknownSubjectError);
    EXPECT_THROW(vtf.get_transform("foo"),
                 vicon_transformer::UnknownSubjectError);
    EXPECT_THROW(vtf.get_raw_transform("foo"),
                 vicon_transformer::UnknownSubjectError);
}

TEST(ViconTransformer, subject_not_visible_error)
{
    ViconTransformer vtf(get_receiver("frame_with_missing_subjects.json"), "");
    vtf.update();

    EXPECT_FALSE(vtf.is_visible("rll_led_stick"));

    EXPECT_THROW(vtf.get_transform("rll_led_stick"),
                 vicon_transformer::SubjectNotVisibleError);
    EXPECT_THROW(vtf.get_raw_transform("rll_led_stick"),
                 vicon_transformer::SubjectNotVisibleError);
}

TEST(ViconTransformer, origin_transform)
{
    // Load two different test frames of the same scene but with the Vicon
    // origin at different locations.  By setting the desired origin to the ping
    // marker, all objects should appear more or less at the same location
    // nonetheless.

    ViconTransformer vtf1(get_receiver("test_frame1.json"), "rll_ping_base");
    ViconTransformer vtf2(get_receiver("test_frame2.json"), "rll_ping_base");
    vtf1.update();
    vtf2.update();

    for (const std::string &subject_name : vtf1.get_subject_names())
    {
        // the marker of the Ballmaschine is not very good, better ignore it
        // here
        if (subject_name == "Marker Ballmaschine")
        {
            continue;
        }

        Transformation tf1 = vtf1.get_transform(subject_name);
        Transformation tf2 = vtf2.get_transform(subject_name);

        EXPECT_TRUE(tf1.translation.isApprox(tf2.translation, 2e-3));
        EXPECT_LE(tf1.rotation.angularDistance(tf2.rotation), 0.02);
    }
}

TEST(ViconTransformer, basic_transforms_with_ping_at_origin)
{
    ViconTransformer vtf(get_receiver("frame_ping_at_origin.json"),
                         "rll_ping_base");
    vtf.update();

    Eigen::Matrix4d expected_rll_muscle_base, expected_corner_1;

    expected_rll_muscle_base << 0.8663438846138151, 0.4993031329659253,
        -0.012027260812682643, 1.0833450422755914,  //
        0.49936305903567846, -0.8663894341721914, 0.002425618543688639,
        0.5051439649956338,  //
        -0.009209172751897504, -0.008107389542971665, -0.9999247278530641,
        0.4685935179506591,  //
        0.0, 0.0, 0.0, 1.0;

    expected_corner_1 << 0.9447796559203805, -0.3274233104572087,
        0.01361534164865755, -1.2825873210084287,  //
        0.32746814862858703, 0.94486152759689, -0.0011424977185985471,
        -1.1358202228673058,  //
        -0.012490532123690789, 0.00553799932409894, 0.9999066542286601,
        0.04067459816726638,  //
        0.0, 0.0, 0.0, 1.0;

    ASSERT_MATRIX_ALMOST_EQUAL(vtf.get_transform("rll_muscle_base").matrix(),
                               expected_rll_muscle_base);
    ASSERT_MATRIX_ALMOST_EQUAL(
        vtf.get_transform("TT Platte_Eckteil 1").matrix(), expected_corner_1);
}

TEST(ViconTransformer, basic_transforms_with_ping_translated)
{
    ViconTransformer vtf(get_receiver("frame_ping_simple_translation.json"),
                         "rll_ping_base");
    vtf.update();

    Eigen::Matrix4d expected_rll_muscle_base, expected_corner_1;

    expected_rll_muscle_base << 0.8663438846138151, 0.4993031329659253,
        -0.012027260812682643, 83.3450422755914 / 1000,  //
        0.49936305903567846, -0.8663894341721914, 0.002425618543688639,
        480.1439649956338 / 1000,  //
        -0.009209172751897504, -0.008107389542971665, -0.9999247278530641,
        471.5935179506591 / 1000,  //
        0.0, 0.0, 0.0, 1.0;

    expected_corner_1 << 0.9447796559203805, -0.3274233104572087,
        0.01361534164865755, -2282.5873210084287 / 1000,  //
        0.32746814862858703, 0.94486152759689, -0.0011424977185985471,
        -1160.8202228673058 / 1000,  //
        -0.012490532123690789, 0.00553799932409894, 0.9999066542286601,
        43.67459816726638 / 1000,  //
        0.0, 0.0, 0.0, 1.0;

    ASSERT_MATRIX_ALMOST_EQUAL(vtf.get_transform("rll_muscle_base").matrix(),
                               expected_rll_muscle_base);
    ASSERT_MATRIX_ALMOST_EQUAL(
        vtf.get_transform("TT Platte_Eckteil 1").matrix(), expected_corner_1);
}
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
