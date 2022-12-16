#!/usr/bin/env python3
"""Use Python SDK to connect to Vicon and forward data via ZMQ.

THIS SCRIPT IS DEPRECATED!  Use bridge_zmq.py instead.
"""
import argparse
import signal
import sys
import time
import typing

import zmq

from vicon_dssdk import ViconDataStream  # noqa


client: ViconDataStream.Client
context: zmq.Context


# handle ctrl+c to disconnect
def signal_handler(sig, frame):
    global client, context

    print("You pressed Ctrl+C! -> closing ")
    client.Disconnect()
    context.destroy()
    print("bye...")
    sys.exit(0)


def main() -> None:
    global client, context

    print("THIS SCRIPT IS DEPRECATED!  Use bridge_zmq.py instead.")

    # parse arguments
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "host",
        nargs="?",
        help="Host name, in the format of server:port",
        default="localhost:801",
    )
    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal_handler)

    # start vicon client
    client = ViconDataStream.Client()
    j: typing.Dict[str, typing.Any] = {"format_version": 2}
    tried = 0
    while tried < 10:
        try:
            print("try num: " + str(tried))
            client.Connect(args.host)

            # Check setting the buffer size works
            client.SetBufferSize(1)

            # Enable all the data types
            client.EnableSegmentData()
            client.EnableMarkerData()
            client.EnableUnlabeledMarkerData()
            client.EnableMarkerRayData()
            client.EnableDeviceData()
            client.EnableCentroidData()

            # establish zmq connection
            context = zmq.Context()
            socket = context.socket(zmq.PUB)
            socket.bind("tcp://10.42.2.29:5555")  #

            num_frame = 0
            start_time = time.time_ns()
            while True:
                on_time = (time.time_ns() - start_time) / 1000000000

                HasFrame = False
                num_frame = num_frame + 1
                if num_frame % 150 == 0:
                    msg = (
                        "frame # "
                        + str(num_frame)
                        + " in "
                        + str(on_time)
                        + " s -> "
                        + str(num_frame / on_time)
                        + " frames/s"
                    )
                    print(msg)

                while not HasFrame:
                    try:
                        client.GetFrame()
                        HasFrame = True
                    except ViconDataStream.DataStreamException:
                        client.GetFrame()

                client.SetStreamMode(ViconDataStream.Client.StreamMode.EServerPush)

                j["frame_number"] = client.GetFrameNumber()
                j["my_frame_number"] = num_frame
                j["on_time"] = (time.time_ns() - start_time) / 1000000000

                j["frame_rate"] = client.GetFrameRate()

                (
                    hours,
                    minutes,
                    seconds,
                    frames,
                    subframe,
                    fieldFlag,
                    standard,
                    subFramesPerFrame,
                    userBits,
                ) = client.GetTimecode()
                j["time_stamp"] = time.time_ns()

                j["latency"] = client.GetLatencyTotal()

                # print( 'Latency Samples' )
                # for sampleName, sampleValue in client.GetLatencySamples().items():
                #     print( sampleName, sampleValue )

                # print( 'Frame Rates' )
                # for frameRateName, frameRateValue in client.GetFrameRates().items():
                #     print( frameRateName, frameRateValue )

                client.SetAxisMapping(
                    ViconDataStream.Client.AxisMapping.EForward,
                    ViconDataStream.Client.AxisMapping.ELeft,
                    ViconDataStream.Client.AxisMapping.EUp,
                )
                xAxis, yAxis, zAxis = client.GetAxisMapping()

                subject_names = client.GetSubjectNames()

                def get_subject_data(
                    subject_name: str, client: ViconDataStream.Client
                ) -> dict:
                    result = {}
                    segment_name = client.GetSubjectRootSegmentName(subject_name)

                    result["global_translation"] = client.GetSegmentGlobalTranslation(
                        subject_name, segment_name
                    )

                    result["global_rotation"] = {
                        "quaternion": client.GetSegmentGlobalRotationQuaternion(
                            subject_name, segment_name
                        )
                    }

                    try:
                        result["quality"] = client.GetObjectQuality(subject_name)
                    except ViconDataStream.DataStreamException:
                        result["quality"] = "Not present"

                    return result

                j["subjects"] = {
                    subject_name: get_subject_data(subject_name, client)
                    for subject_name in subject_names
                }

                # broadcast json object
                socket.send_json(j)

        except ViconDataStream.DataStreamException as e:
            print("Handled data stream error", e)
            tried = tried + 1


if __name__ == "__main__":
    main()
