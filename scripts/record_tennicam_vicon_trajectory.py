#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Record a trajectory with both tennicam and Vicon in parallel.

Assumes a ball is attached to the tip of the LED stick, such that the trajectory
of the ball can be recorded via both tennicam (tracking the ball) and Vicon
(tracking the LED stick).

The recorded trajectory can then be used to compute a transformation between
tennicam and Vicon.

Things to consider when recording:
- Make sure the ball is centred exactly on the tip of the LED stick, so that
  tennicam and Vicon measure the same position.
- Move slowly.  Tennicam and Vicon are not exactly synchronised, so better move
  the ball/stick slowly, to reduce errors introduced by this.

Stop the recording by pressing Ctrl+C.  The trajectory is then saved to the specified
destination path using JSON format.  It is structured as sequence of objects with the
values
- "tennicam_position": Position [x, y, z] of the ball as detected by tennicam.
- "tennicam_timestamp": Timestamp of the tennicam position.
- "vicon_position:": Position [x, y, z] of the LED stick as detected by tennicam.
- "vicon_timestamp": Timestamp of the Vicon position.
"""
import argparse
import contextlib
import logging
import json
import pathlib
import sys
import time
import typing as t

import numpy as np
import tqdm

import signal_handler
import tennicam_client
from vicon_transformer import pam_vicon_o80


class JsonEncoder(json.JSONEncoder):
    """JSON encoder that handles custom types used here"""

    def default(self, obj: t.Any) -> t.Any:
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        return json.JSONEncoder.default(self, obj)


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "destination",
        type=pathlib.Path,
        help="File to which the recorded data is written.",
    )
    parser.add_argument(
        "--rate",
        "-r",
        type=float,
        default="1.0",
        metavar="SECONDS",
        help="Rate in seconds at which positions are saved.  Default: %(default)s",
    )
    parser.add_argument(
        "--tennicam-segment-id",
        type=str,
        default="tennicam_client",
        help="""Shared memory segment ID used by the tennicam back end.
            Default: "%(default)s"
        """,
    )
    parser.add_argument(
        "--vicon-segment-id",
        type=str,
        default="vicon",
        help="""Shared memory segment ID used by the vicon back end.
            Default: "%(default)s"
        """,
    )
    parser.add_argument(
        "--verbose", "-v", action="store_true", help="Enable verbose output."
    )
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.DEBUG if args.verbose else logging.INFO,
        format="[%(asctime)s] [%(name)s | %(levelname)s] %(message)s",
    )

    if args.destination.exists():
        logging.fatal("Destination file %s already exists.", args.destination)
        return 1

    logging.debug("Tennicam segment id: %s", args.tennicam_segment_id)
    logging.debug("Vicon segment id: %s", args.vicon_segment_id)
    logging.debug("Recording rate: %f seconds", args.rate)

    tennicam_frontend = tennicam_client.FrontEnd(args.tennicam_segment_id)
    vicon_frontend = pam_vicon_o80.FrontEnd(args.vicon_segment_id)

    data = []
    signal_handler.init()  # for detecting ctrl+c
    with contextlib.suppress(KeyboardInterrupt, SystemExit), tqdm.tqdm() as progress:
        next_update = time.time() + args.rate
        while not signal_handler.has_received_sigint():
            now = time.time()
            sleep_duration = max(0, next_update - now)
            time.sleep(sleep_duration)
            next_update += args.rate

            # Tennicam is a bit slower than Vicon (~180 Hz vs 300 Hz).  Therefore, wait
            # for the next tennicam observation and then immediately get the latest
            # Vicon observation.  This way, they should be fairly synchronised.
            logging.debug("tennicam: Wait for next observation.")
            i = tennicam_frontend.latest().get_iteration()
            try:
                obs_tennicam = tennicam_frontend.read(i + 1)
            except Exception:
                # Only print error message if the exception was not caused by a SIGINT
                # signal.  In either case, break the loop so that recorded data can be
                # saved to file before terminating.
                if not signal_handler.has_received_sigint():
                    logging.exception("Unexpected error occurred. Abort recording.")
                break

            logging.debug("vicon: Get latest observation.")
            obs_vicon = vicon_frontend.latest()

            vicon_frame = obs_vicon.get_extended_state()
            led_stick = vicon_frame.subjects[pam_vicon_o80.Subjects.LED_STICK]

            if obs_tennicam.get_ball_id() < 0:
                logging.error("No ball detected.  Skip frame.")
                continue

            if not led_stick.is_visible:
                logging.error("LED stick not detected.  Skip frame.")
                continue

            position_tennicam = obs_tennicam.get_position()
            position_vicon = led_stick.global_pose.translation

            data.append(
                {
                    "tennicam_timestamp": obs_tennicam.get_time_stamp(),
                    "tennicam_position": position_tennicam,
                    "vicon_timestamp": vicon_frame.time_stamp,
                    "vicon_position": position_vicon,
                }
            )
            progress.update()

    logging.info("Save trajectory to file %s", args.destination)
    with open(args.destination, "w") as f:
        json.dump(data, f, cls=JsonEncoder)

    return 0


if __name__ == "__main__":
    with contextlib.suppress(KeyboardInterrupt):
        sys.exit(main())
