/**
 * @file
 * @brief Tests for vicon_receiver.hpp
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <gtest/gtest.h>

#include <vicon_transformer/vicon_receiver.hpp>

using vicon_transformer::JsonReceiver;
using vicon_transformer::ViconFrame;

TEST(JsonReceiver, load_file)
{
    // assumes test is executed in package root directory
    std::string file = "tests/data/test_frame1_format3.json";

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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
