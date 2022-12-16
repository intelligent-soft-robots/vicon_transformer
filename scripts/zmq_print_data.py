#!/usr/bin/env python3
"""Receive JSON date via ZMQ and print to terminal."""
import argparse
import json
import logging
import signal
import sys

import zmq  # type: ignore


# handle ctrl+c to disconnect
def signal_handler(sig, frame):
    print("You pressed Ctrl+C! -> closing ")
    sys.exit(0)


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
    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal_handler)

    logging.info("connecting to %s...", args.address)
    with zmq.Context() as context:
        sub = context.socket(zmq.SUB)
        sub.setsockopt(zmq.SUBSCRIBE, b"")
        sub.RCVTIMEO = 5000  # wait only 5s for new message
        sub.connect(args.address)  # Note
        logging.info("connected")

        logging.info("start receiving")
        while True:
            j = sub.recv_json()
            print(json.dumps(j, indent=4, sort_keys=True))

    return 0


if __name__ == "__main__":
    sys.exit(main())
