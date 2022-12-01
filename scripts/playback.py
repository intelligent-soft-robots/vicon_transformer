#!/usr/bin/env python3
"""Play back Vicon data from a previously recorded pickle file."""
import argparse
import logging
import pathlib
import pickle
import sys
import time

import tqdm
import zmq  # type: ignore


def main():
    logging.basicConfig(level=logging.INFO)

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "address",
        type=str,
        nargs="?",
        default="tcp://10.42.2.29:5555",
        help="Address to connect the zmq socket to.  Default: %(default)s.",
    )
    parser.add_argument(
        "input_file", type=pathlib.Path, help="File with the recorded data."
    )
    args = parser.parse_args()

    try:
        with open(args.input_file, "rb") as f:
            data = pickle.load(f)
    except FileNotFoundError:
        logging.fatal("File %s does not exists.", args.input_file)
        return 1
    except Exception as e:
        logging.fatal("Failed to read file %s.  Error: %s", args.input_file, e)
        return 1

    with zmq.Context() as context:
        with context.socket(zmq.PUB) as socket:
            logging.info("Publish to %s", args.address)
            socket.bind(args.address)

            logging.info("start playback")
            stamp_0 = data[0]["time_stamp"]
            now = time.time_ns()
            time_offset = now - stamp_0

            for frame in tqdm.tqdm(data):
                # wait until next frame is due
                now = time.time_ns()
                while now - time_offset <= frame["time_stamp"]:
                    time.sleep(0.001)
                    now = time.time_ns()
                socket.send_json(frame)

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        logging.info("You pressed Ctrl+C -> stop playback")
        sys.exit(2)
