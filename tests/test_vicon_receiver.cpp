/**
 * @file
 * @brief Tests for vicon_receiver.hpp
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <gtest/gtest.h>

#include <serialization_utils/cereal_json.hpp>

#include <vicon_transformer/vicon_receiver.hpp>

#include "utils.hpp"

using vicon_transformer::JsonReceiver;
using vicon_transformer::PlaybackReceiver;
using vicon_transformer::ViconFrame;

TEST(JsonReceiver, load_file)
{
    // assumes test is executed in package root directory
    std::string file = "tests/data/test_frame1.json";

    JsonReceiver receiver(file);
    ViconFrame frame = receiver.read();

    // check that the frame's data matches with test_frame1.json
    EXPECT_EQ(frame.time_stamp, 1638538681615901200);
    EXPECT_EQ(frame.frame_number, 408812);
}

TEST(JsonReceiver, file_not_found)
{
    // assumes test is executed in package root directory
    std::string file = "tests/data/this_does_not_exist.json";

    EXPECT_THROW({ JsonReceiver receiver(file); }, std::runtime_error);
}

TEST(ViconFrame, serialize)
{
    // assumes test is executed in package root directory
    std::string file = "tests/data/test_frame1.json";

    JsonReceiver receiver(file);
    ViconFrame frame1 = receiver.read();

    // serialize and deserialize (use json helper functions for convenience)
    std::string json = serialization_utils::to_json(frame1);
    auto frame2 = serialization_utils::from_json<ViconFrame>(json);

    // verify frame gets deserialized to original values
    EXPECT_EQ(frame1.frame_number, frame2.frame_number);
    EXPECT_EQ(frame1.frame_rate, frame2.frame_rate);
    EXPECT_EQ(frame1.time_stamp, frame2.time_stamp);
    EXPECT_EQ(frame1.latency, frame2.latency);
    vicon_transformer::SubjectData arm1 = frame1.subjects.at("Marker_Arm");
    vicon_transformer::SubjectData arm2 = frame2.subjects.at("Marker_Arm");
    EXPECT_EQ(arm1.is_visible, arm2.is_visible);
    EXPECT_EQ(arm1.quality, arm2.quality);
    ASSERT_QUATERNION_ALMOST_EQUAL(arm1.global_pose.rotation,
                                   arm2.global_pose.rotation);
    ASSERT_MATRIX_ALMOST_EQUAL(arm1.global_pose.translation,
                               arm2.global_pose.translation);
}

TEST(PlaybackReceiver, load_and_playback)
{
    // assumes test is executed in package root directory
    std::string file = "tests/data/recording_3s.dat";

    PlaybackReceiver receiver(file);

    // verify file is loaded by checking some values of the first frame
    ViconFrame frame = receiver.read();

    EXPECT_EQ(frame.frame_number, 6294704);
    EXPECT_EQ(frame.time_stamp, 1673885215803208651);
    EXPECT_EQ(frame.latency, 0.010578898712992668);
    EXPECT_EQ(frame.subjects["Marker_Arm"].quality, 3.02566717326428);

    // The recording takes ~3 seconds.  At 300 fps, this corresponds to ~900
    // frames. Loop over them and verify the proper exception is thrown in the
    // end.
    for (int i = 0; i < 890; i++)
    {
        receiver.read();
    }

    // when reaching the end, there should be a std::out_of_range error
    EXPECT_THROW(
        {
            for (int i = 0; i < 20; i++)
            {
                receiver.read();
            }
        },
        std::out_of_range);
}

TEST(PlaybackReceiver, file_not_found)
{
    std::string file = "tests/data/this_does_not_exists.dat";
    EXPECT_THROW({ PlaybackReceiver receiver(file); }, std::runtime_error);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
