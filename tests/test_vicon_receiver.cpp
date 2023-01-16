/**
 * @file
 * @brief Tests for vicon_receiver.hpp
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <gtest/gtest.h>

#include <vicon_transformer/vicon_receiver.hpp>

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
