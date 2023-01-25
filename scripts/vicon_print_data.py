#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""Get data from Vicon server and forward with ZMQ."""
import argparse
import sys

from vicon_transformer import ViconReceiverConfig, ViconReceiver, to_json


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
    parser.add_argument(
        "--json", action="store_true", help="Print output in JSON format."
    )
    args = parser.parse_args()

    config = ViconReceiverConfig()
    config.enable_lightweight = args.lightweight
    config.filtered_subjects = args.subjects or []

    print("Config:\n{}\n".format(to_json(config)))

    with ViconReceiver(args.vicon_host, config) as receiver:
        receiver.print_info()

        print("=======================================")

        while True:
            frame = receiver.read()
            receiver.print_latency_info()

            if args.json:
                print(to_json(frame))
            else:
                print(frame)

            if args.once:
                break

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        pass
