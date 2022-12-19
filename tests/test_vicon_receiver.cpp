/**
 * @file
 * @brief Tests for vicon_receiver.hpp
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#ifndef TEST_DATA_FILE_DIR
#error TEST_DATA_FILE_DIR is not defined
#endif

#include <gtest/gtest.h>

#include <vicon_transformer/vicon_receiver.hpp>

using vicon_transformer::JsonReceiver;
using vicon_transformer::ViconFrame;

TEST(JsonReceiver, load_file)
{
    std::string file = std::string(TEST_DATA_FILE_DIR) + "/test_frame1_format3.json";

    JsonReceiver receiver(file);
    ViconFrame frame = receiver.read();

    // check that the frame's data matches with test_frame1.json
    ASSERT_EQ(frame.time_stamp, 1638538681615901200);
    ASSERT_EQ(frame.frame_number, 408812);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
