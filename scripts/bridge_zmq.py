#!/usr/bin/env python3
"""Get data from Vicon server and forward with ZMQ."""
import argparse
import sys
import logging

import zmq

from vicon_transformer import ViconReceiverConfig, ViconReceiver, ViconFrame


def frame_to_dict_format2(frame: ViconFrame) -> dict:
    frame_dict = {
        "format_version": 2,
        "frame_number": frame.frame_number,
        "latency": frame.latency,
        "time_stamp": frame.time_stamp,
        "my_frame_number": 0,  # field is not supported
        "on_time": 0.0,  # field is not supported
        "subjects": {},
    }

    for name, data in frame.subjects.items():
        frame_dict["subjects"][name] = {
            "global_translation": [data.global_translation, not data.is_visible],
            "global_rotation": [data.global_rotation_quaternion, not data.is_visible],
            "quality": data.quality,
        }

    return frame_dict


def main():
    # parse arguments
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "vicon_host",
        nargs="?",
        help="Host name, in the format of server:port",
        default="localhost:801",
    )
    parser.add_argument("zmq_address", type=str, help="Address for the ZMQ publisher.")
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO)

    config = ViconReceiverConfig()
    receiver = ViconReceiver(args.vicon_host, config)

    # TODO: simple wrapper around ViconReceiver with context manager?
    receiver.connect()
    receiver.print_info()

    with zmq.Context() as context, context.socket(zmq.PUB) as socket:
        logging.info("Publish to %s", args.zmq_address)
        socket.bind(args.zmq_address)

        while True:
            frame = receiver.read()
            frame_dict = frame_to_dict_format2(frame)
            socket.send_json(frame_dict)

    return 0


if __name__ == "__main__":
    sys.exit(main())
