#!/usr/bin/env python3
"""Get data from Vicon server and forward with ZMQ."""
import argparse
import sys

from vicon_transformer import ViconReceiverConfig, ViconReceiver


def main():
    # parse arguments
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "vicon_host",
        nargs="?",
        help="Host name, in the format of server:port",
        default="localhost:801",
    )
    parser.add_argument(
        "--lightweight",
        action="store_true",
        help="Enable lightweight segment data.",
    )
    parser.add_argument(
        "--subjects",
        type=str,
        nargs="+",
        metavar="SUBJECT",
        help="""Filter for listed subjects.  Other subjects are still listed in the
            frame data but without actual data.
        """,
    )
    parser.add_argument(
        "--once", action="store_true", help="Exit after printing one frame."
    )
    args = parser.parse_args()

    config = ViconReceiverConfig()
    config.enable_lightweight = args.lightweight
    receiver = ViconReceiver(args.vicon_host, config)

    # TODO: simple wrapper around ViconReceiver with context manager?
    receiver.connect()
    if args.subjects:
        receiver.filter_subjects(args.subjects)

    receiver.print_info()

    print("=======================================")

    while True:
        frame = receiver.read()
        receiver.print_latency_info()
        print(frame)

        if args.once:
            break

    return 0


if __name__ == "__main__":
    sys.exit(main())
