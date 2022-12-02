from __future__ import print_function

import argparse
import signal
import sys
import time
from os.path import dirname, abspath, join

import zmq

# Find code directory relative to our directory
THIS_DIR = dirname(__file__)
CODE_DIR = abspath(join(THIS_DIR, "..", "vicon_dssdk"))
sys.path.append(CODE_DIR)
from vicon_dssdk import ViconDataStream  # noqa


# parse arguments
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument(
    "host",
    nargs="?",
    help="Host name, in the format of server:port",
    default="localhost:801",
)
args = parser.parse_args()


# handle ctrl+c to disconnect
def signal_handler(sig, frame):
    print("You pressed Ctrl+C! -> closing ")
    client.Disconnect()
    print("zmq closed? " + str(socket.closed))
    context.destroy()
    print("zmq closed? " + str(socket.closed))
    print("bye...")
    sys.exit(0)


signal.signal(signal.SIGINT, signal_handler)

# start vicon client
client = ViconDataStream.Client()
j = {}
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
            frame_time = time.time_ns()
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

            # Try setting the different stream modes
            client.SetStreamMode(ViconDataStream.Client.StreamMode.EClientPull)
            # print( 'Get Frame Pull', client.GetFrame(), client.GetFrameNumber() )

            client.SetStreamMode(ViconDataStream.Client.StreamMode.EClientPullPreFetch)
            # print( 'Get Frame PreFetch', client.GetFrame(), client.GetFrameNumber() )

            client.SetStreamMode(ViconDataStream.Client.StreamMode.EServerPush)
            # print( 'Get Frame Push', client.GetFrame(), client.GetFrameNumber() )
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

            # print( 'Server Orientation', client.GetServerOrientation() )

            subjectNames = client.GetSubjectNames()
            j["subjectNames"] = subjectNames
            j["num_subjects"] = len(subjectNames)
            sbj_idx = 0
            for subjectName in subjectNames:
                sbj_idx_str = "subject_" + str(sbj_idx)
                sbj_idx = sbj_idx + 1

                segmentNames = client.GetSegmentNames(subjectName)
                for segmentName in segmentNames:
                    segmentChildren = client.GetSegmentChildren(
                        subjectName, segmentName
                    )

                    j[sbj_idx_str] = {}
                    j[sbj_idx_str]["name"] = subjectName
                    j[sbj_idx_str]["global_translation"] = {}
                    j[sbj_idx_str][
                        "global_translation"
                    ] = client.GetSegmentGlobalTranslation(subjectName, segmentName)
                    j[sbj_idx_str]["global_rotation"] = {}
                    j[sbj_idx_str]["global_rotation"][
                        "helical"
                    ] = client.GetSegmentGlobalRotationHelical(subjectName, segmentName)
                    j[sbj_idx_str]["global_rotation"][
                        "eulerxyz"
                    ] = client.GetSegmentGlobalRotationEulerXYZ(
                        subjectName, segmentName
                    )
                    j[sbj_idx_str]["global_rotation"][
                        "quaternion"
                    ] = client.GetSegmentGlobalRotationQuaternion(
                        subjectName, segmentName
                    )
                    j[sbj_idx_str]["global_rotation"][
                        "matrix"
                    ] = client.GetSegmentGlobalRotationMatrix(subjectName, segmentName)
                try:
                    j[sbj_idx_str]["quality"] = client.GetObjectQuality(subjectName)
                except ViconDataStream.DataStreamException:
                    j[sbj_idx_str]["quality"] = "Not present"

            # broadcast json object
            socket.send_json(j)

    except ViconDataStream.DataStreamException as e:
        print("Handled data stream error", e)
        tried = tried + 1
