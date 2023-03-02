#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Connect to a shared memory set up by an o80 back end and print the received data.

The back end is expected to be run in a separate process (e.g. by
``vicon_o80_standalone``) using the same segment ID.  It needs to be started before
``vicon_o80_print_data``.
"""
import argparse
import logging
import sys
import typing

import signal_handler

from vicon_transformer.pam_vicon_o80 import (
    FrontEnd,
    FixedSizeViconFrame,
    Subjects,
    get_subject_names,
)


DEFAULT_SEGMENT_ID = "vicon"


def print_frame_data(
    frame: FixedSizeViconFrame, subject_names: typing.Sequence[str]
) -> None:
    """Print Vicon frame data including subject names.

    Args:
        frame: The Vicon frame received through o80.
        subject_names: List of subject names in same order as ``frame.subjects``.
    """
    print("Frame Number: {}".format(frame.frame_number))
    print("Frame Rate: {}".format(frame.frame_rate))
    print("Latency: {}".format(frame.latency))
    print("Timestamp: {}".format(frame.time_stamp))
    print("Subjects ({}):".format(len(frame.subjects)))
    for i, subject in enumerate(frame.subjects):
        print("  {}:".format(Subjects(i).name))
        print("    Name: {}".format(subject_names[i]))
        print("    Visible: {}".format(subject.is_visible))
        print("    Translation: {}".format(subject.global_pose.translation))
        print("    Rotation: {}".format(subject.global_pose.get_rotation()))
        print("    Quality: {}".format(subject.quality))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--segment-id",
        "-s",
        type=str,
        default=DEFAULT_SEGMENT_ID,
        help="Shared memory segment ID. Default: '%(default)s'",
    )
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO)

    frontend = FrontEnd(args.segment_id)
    iteration = frontend.latest().get_iteration()

    subject_names = get_subject_names()

    signal_handler.init()  # for detecting ctrl+c
    try:
        while not signal_handler.has_received_sigint():
            iteration += 1
            obs = frontend.read(iteration)
            vicon_frame = obs.get_extended_state()

            print("----")
            print_frame_data(vicon_frame, subject_names)

    except (KeyboardInterrupt, SystemExit):
        pass
    except Exception as e:
        print("Error:", e)

    return 0


if __name__ == "__main__":
    sys.exit(main())
