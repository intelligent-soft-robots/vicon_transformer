from __future__ import print_function

from os.path import dirname, abspath, join
import sys
import zmq
import json
import time

# Find code directory relative to our directory
THIS_DIR = dirname(__file__)
CODE_DIR = abspath(join(THIS_DIR, "..", "vicon_dssdk"))
sys.path.append(CODE_DIR)
from vicon_dssdk import ViconDataStream


import signal
import sys

# parse arguments
import argparse

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

        # Check the version
        # print( 'Version', client.GetVersion() )

        # Check setting the buffer size works
        client.SetBufferSize(1)

        # Enable all the data types
        client.EnableSegmentData()
        client.EnableMarkerData()
        client.EnableUnlabeledMarkerData()
        client.EnableMarkerRayData()
        client.EnableDeviceData()
        client.EnableCentroidData()

        # Report whether the data types have been enabled
        # print( 'Segments', client.IsSegmentDataEnabled() )
        # print( 'Markers', client.IsMarkerDataEnabled() )
        # print( 'Unlabeled Markers', client.IsUnlabeledMarkerDataEnabled() )
        # print( 'Marker Rays', client.IsMarkerRayDataEnabled() )
        # print( 'Devices', client.IsDeviceDataEnabled() )
        # print( 'Centroids', client.IsCentroidDataEnabled() )

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
                except ViconDataStream.DataStreamException as e:
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

            # print( 'Frame Rate', client.GetFrameRate() )
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
            # print( ('Timecode:', hours, 'hours', minutes, 'minutes', seconds, 'seconds', frames,
            #     'frames', subframe, 'sub frame', fieldFlag, 'field flag',
            #     standard, 'standard', subFramesPerFrame, 'sub frames per frame', userBits, 'user bits') )
            j["time_stamp"] = time.time_ns()

            # print( 'Total Latency', client.GetLatencyTotal() )
            j["latency"] = client.GetLatencyTotal()

            # print( 'Latency Samples' )
            # for sampleName, sampleValue in client.GetLatencySamples().items():
            # print( sampleName, sampleValue )

            # print( 'Frame Rates' )
            # for frameRateName, frameRateValue in client.GetFrameRates().items():
            # print( frameRateName, frameRateValue )

            # try:
            #     client.SetApexDeviceFeedback( 'BogusDevice', True )
            # except ViconDataStream.DataStreamException as e:
            #     print( 'No Apex Devices connected' )

            client.SetAxisMapping(
                ViconDataStream.Client.AxisMapping.EForward,
                ViconDataStream.Client.AxisMapping.ELeft,
                ViconDataStream.Client.AxisMapping.EUp,
            )
            xAxis, yAxis, zAxis = client.GetAxisMapping()
            # print( 'X Axis', xAxis, 'Y Axis', yAxis, 'Z Axis', zAxis )

            # print( 'Server Orientation', client.GetServerOrientation() )

            # try:
            #     client.SetTimingLog( '', '' )
            # except ViconDataStream.DataStreamException as e:
            #     print( 'Failed to set timing log' )

            # try:
            #     client.ConfigureWireless()
            # except ViconDataStream.DataStreamException as e:
            #     print( 'Failed to configure wireless', e )

            subjectNames = client.GetSubjectNames()
            j["subjectNames"] = subjectNames
            j["num_subjects"] = len(subjectNames)
            sbj_idx = 0
            for subjectName in subjectNames:
                # print( subjectName )
                sbj_idx_str = "subject_" + str(sbj_idx)
                sbj_idx = sbj_idx + 1

                segmentNames = client.GetSegmentNames(subjectName)
                for segmentName in segmentNames:
                    segmentChildren = client.GetSegmentChildren(
                        subjectName, segmentName
                    )
                    # for child in segmentChildren:
                    # try:
                    # print( child, 'has parent', client.GetSegmentParentName( subjectName, segmentName ) )
                    # except ViconDataStream.DataStreamException as e:
                    # print( 'Error getting parent segment', e )
                    # print( segmentName, 'has static translation', client.GetSegmentStaticTranslation( subjectName, segmentName ) )
                    # print( segmentName, 'has static rotation( helical )', client.GetSegmentStaticRotationHelical( subjectName, segmentName ) )
                    # print( segmentName, 'has static rotation( EulerXYZ )', client.GetSegmentStaticRotationEulerXYZ( subjectName, segmentName ) )
                    # print( segmentName, 'has static rotation( Quaternion )', client.GetSegmentStaticRotationQuaternion( subjectName, segmentName ) )
                    # print( segmentName, 'has static rotation( Matrix )', client.GetSegmentStaticRotationMatrix( subjectName, segmentName ) )
                    # try:
                    # print( segmentName, 'has static scale', client.GetSegmentStaticScale( subjectName, segmentName ) )
                    # except ViconDataStream.DataStreamException as e:
                    # print( 'Scale Error', e )
                    # print( segmentName, 'has global translation', client.GetSegmentGlobalTranslation( subjectName, segmentName ) )
                    # print( segmentName, 'has global rotation( helical )', client.GetSegmentGlobalRotationHelical( subjectName, segmentName ) )
                    # print( segmentName, 'has global rotation( EulerXYZ )', client.GetSegmentGlobalRotationEulerXYZ( subjectName, segmentName ) )
                    # print( segmentName, 'has global rotation( Quaternion )', client.GetSegmentGlobalRotationQuaternion( subjectName, segmentName ) )
                    # print( segmentName, 'has global rotation( Matrix )', client.GetSegmentGlobalRotationMatrix( subjectName, segmentName ) )
                    # print( segmentName, 'has local translation', client.GetSegmentLocalTranslation( subjectName, segmentName ) )
                    # print( segmentName, 'has local rotation( helical )', client.GetSegmentLocalRotationHelical( subjectName, segmentName ) )
                    # print( segmentName, 'has local rotation( EulerXYZ )', client.GetSegmentLocalRotationEulerXYZ( subjectName, segmentName ) )
                    # print( segmentName, 'has local rotation( Quaternion )', client.GetSegmentLocalRotationQuaternion( subjectName, segmentName ) )
                    # print( segmentName, 'has local rotation( Matrix )', client.GetSegmentLocalRotationMatrix( subjectName, segmentName ) )
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
                    # print( 'Object Quality', client.GetObjectQuality( subjectName ) )
                    j[sbj_idx_str]["quality"] = client.GetObjectQuality(subjectName)
                except ViconDataStream.DataStreamException as e:
                    # print( 'Not present', e )
                    j[sbj_idx_str]["quality"] = "Not present"

            # broadcast json object
            socket.send_json(j)

            # print(json.dumps(j, indent=4, sort_keys=True))
            # markerNames = client.GetMarkerNames( subjectName )
            # for markerName, parentSegment in markerNames:
            #     print( markerName, 'has parent segment', parentSegment, 'position', client.GetMarkerGlobalTranslation( subjectName, markerName ) )
            #     rayAssignments = client.GetMarkerRayAssignments( subjectName, markerName )
            #     if len( rayAssignments ) == 0:
            #         print( 'No ray assignments for', markerName )
            #     else:
            #         for cameraId, centroidIndex in rayAssignments:
            #             print( 'Ray from', cameraId, 'centroid', centroidIndex )

            # unlabeledMarkers = client.GetUnlabeledMarkers()
            # for markerPos, trajID in unlabeledMarkers:
            #     print( 'Unlabeled Marker at', markerPos, 'with trajID', trajID )

            # labeledMarkers = client.GetLabeledMarkers()
            # for markerPos, trajID in labeledMarkers:
            #     print( 'Labeled Marker at', markerPos, 'with trajID', trajID )

            # devices = client.GetDeviceNames()
            # for deviceName, deviceType in devices:
            #     print( deviceName, 'Device of type', deviceType )
            #     deviceOutputDetails = client.GetDeviceOutputDetails( deviceName )
            #     for outputName, componentName, unit in deviceOutputDetails:
            #         values, occluded = client.GetDeviceOutputValues( deviceName, outputName, componentName )
            #         print( deviceName, componentName, values, unit, occluded )

            # forceplates = client.GetForcePlates()
            # for plate in forceplates:
            #     forceVectorData = client.GetForceVector( plate )
            #     momentVectorData = client.GetMomentVector( plate )
            #     copData = client.GetCentreOfPressure( plate )
            #     globalForceVectorData = client.GetGlobalForceVector( plate )
            #     globalMomentVectorData = client.GetGlobalMomentVector( plate )
            #     globalCopData = client.GetGlobalCentreOfPressure( plate )

            #     try:
            #         analogData = client.GetAnalogChannelVoltage( plate )
            #     except ViconDataStream.DataStreamException as e:
            #         print( 'Failed getting analog channel voltages' )

            # eyeTrackers = client.GetEyeTrackers()
            # for eyeTracker in eyeTrackers:
            #     position, occluded = client.GetEyeTrackerGlobalPosition( eyeTracker )
            #     gaze, occluded = client.GetEyeTrackerGlobalGazeVector( eyeTracker )
            #     print( 'Eye Tracker', gaze, position )

            # cameras = client.GetCameraNames()

            # for camera in cameras:
            #     id = client.GetCameraID( camera )
            #     userId = client.GetCameraUserID( camera )
            #     type = client.GetCameraType( camera )
            #     displayName = client.GetCameraDisplayName( camera )
            #     resX, resY = client.GetCameraResolution( camera )
            #     isVideo = client.GetIsVideoCamera( camera )
            #     centroids = client.GetCentroids( camera )
            #     print( id, userId, type, displayName, resX, resY, isVideo )
            #     for centroid, radius, weight in centroids:
            #         print( centroid, radius, weight )

    except ViconDataStream.DataStreamException as e:
        print("Handled data stream error", e)
        tried = tried + 1
