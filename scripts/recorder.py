#!/usr/bin/env python3
"""Record data received from the Vicon system and save to a pickle file."""
import argparse
import logging
import pathlib
import pickle
import signal
import sys
import time
import typing

import tqdm
import zmq  # type: ignore


class DurationEnd(Exception):
    ...


class ZmqRecorder:
    def __init__(self):
        self.stop_requested = False

    def _signal_handler(self, sig, frame):
        logging.info("You pressed Ctrl+C -> stop recording")
        self.stop_requested = True

    def record(
        self, address: str, output_file: pathlib.Path, duration: typing.Optional[float]
    ) -> None:
        self.stop_requested = False

        signal.signal(signal.SIGINT, self._signal_handler)

        logging.info("Connect to %s", address)
        context = zmq.Context()
        sub = context.socket(zmq.SUB)
        sub.setsockopt(zmq.SUBSCRIBE, b"")
        sub.RCVTIMEO = 5000  # wait only 5s for new message
        sub.connect(address)

        if duration:
            print("Start recording for %.0f seconds." % duration)
        else:
            print("Start recording.  Press Ctrl+C to stop.")

        t_start = time.time()
        data = []
        try:
            with tqdm.tqdm() as progress:
                while not self.stop_requested:
                    j = sub.recv_json()
                    data.append(j)
                    progress.update()

                    if duration and (time.time() - t_start) > duration:
                        raise DurationEnd()

        except DurationEnd:
            logging.info("Stop recording after %.1f seconds.", duration)

        except Exception as e:
            logging.error("Error [%s]: %s", type(e), e)
            logging.info("Stop recording.")

        context.destroy()

        record_duration_s = (data[-1]["time_stamp"] - data[0]["time_stamp"]) / 1e9
        logging.info(
            "Save %d frames (%.1f seconds) to %s",
            len(data),
            record_duration_s,
            output_file,
        )
        with open(output_file, "wb") as f:
            pickle.dump(data, f, pickle.HIGHEST_PROTOCOL)


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
    parser.add_argument("output_file", type=pathlib.Path, help="Output destination.")
    parser.add_argument(
        "--duration",
        "-d",
        type=float,
        metavar="SECONDS",
        help="Automatically stop recording after the specified number of seconds.",
    )
    args = parser.parse_args()

    if args.output_file.exists():
        logging.fatal("File %s already exists.", args.output_file)
        return 1

    recorder = ZmqRecorder()
    recorder.record(**vars(args))

    return 0


if __name__ == "__main__":
    sys.exit(main())
