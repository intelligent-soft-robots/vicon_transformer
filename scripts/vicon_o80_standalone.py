#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Run o80 Standalone instance for Vicon data.

This serves as backend to which a FrontEnd instance can connect by using the same
segment ID.
"""
import argparse
import logging
import pathlib
import sys
import time

import o80
import signal_handler

from vicon_transformer.vicon_transformer_bindings import (
    PlaybackReceiver,
    ViconReceiver,
    ViconReceiverConfig,
    to_json,
)
from vicon_transformer.pam_vicon_o80 import (
    start_standalone,
    stop_standalone,
)


DEFAULT_SEGMENT_ID = "vicon"
DEFAULT_FREQUENCY = 300.0


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "host_or_file",
        type=str,
        help="Hostname of Vicon server or path to recorded file.",
    )
    parser.add_argument(
        "--segment-id",
        "-s",
        type=str,
        default=DEFAULT_SEGMENT_ID,
        help="Shared memory segment ID.  Default: '%(default)s'",
    )
    parser.add_argument(
        "--frequency", "-f", type=float, default=DEFAULT_FREQUENCY, help="Frequency."
    )
    parser.add_argument(
        "--burst",
        action="store_true",
        help="Run in bursting mode.  --frequency is ignored in this case.",
    )
    parser.add_argument(
        "--lightweight",
        action="store_true",
        help="""Enable 'lightweight mode' of Vicon.  Saves bandwidth by reducing
            precision of subject poses.
        """,
    )
    parser.add_argument(
        "--subjects",
        type=str,
        nargs="+",
        metavar="SUBJECT",
        help="""Filter for listed subjects.  Other subjects are still included in the
            frame data but without actual data (saves bandwidth).
        """,
    )

    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO)

    if (filename := pathlib.Path(args.host_or_file)).is_file():
        receiver = PlaybackReceiver(filename)
    else:
        config = ViconReceiverConfig()
        config.enable_lightweight = args.lightweight
        config.filtered_subjects = args.subjects or []
        logging.info("Config:\n%s", to_json(config))

        receiver = ViconReceiver(args.host_or_file, config)
        receiver.connect()

    logging.info("Clearning shared memory on '%s'.", args.segment_id)
    o80.clear_shared_memory(args.segment_id)

    logging.info("Starting o80 standalone with frequency %f Hz.", args.frequency)

    start_standalone(
        args.segment_id, args.frequency, args.burst, receiver, "rll_ping_base"
    )

    logging.info("Running, Ctrl+C for exiting")
    signal_handler.init()  # for detecting ctrl+c
    try:
        while not signal_handler.has_received_sigint():
            time.sleep(0.01)
    except (KeyboardInterrupt, SystemExit):
        logging.info("exiting ...")
    except Exception:
        logging.exception("Unexpected error")

    logging.info("Stopping o80 standalone...")
    stop_standalone(args.segment_id)
    if hasattr(receiver, "disconnect"):
        receiver.disconnect()

    logging.info("exiting")
    return 0


if __name__ == "__main__":
    sys.exit(main())
