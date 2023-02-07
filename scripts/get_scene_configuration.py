#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
"""
Get data from Vicon server and print poses of robot, table and ball machine in JSON
format.
"""
import argparse
import contextlib
import json
import logging
import pathlib
import sys
import typing as t

import numpy as np
from scipy.spatial.transform import Rotation

from vicon_transformer import (
    PlaybackReceiver,
    ViconReceiverConfig,
    ViconReceiver,
    ViconTransformer,
)
from vicon_transformer.vicon_transformer_bindings import Transformation, Receiver


ROBOT_BASE_SUBJECT = "rll_muscle_base"
BALL_MACHINE_SUBJECT = "Marker Ballmaschine"
TABLE_CORNER_SUBJECTS = (
    "TT Platte_Eckteil 1",
    "TT Platte_Eckteil 2",
    "TT Platte_Eckteil 3",
    "TT Platte_Eckteil 4",
)


class JsonEncoder(json.JSONEncoder):
    """JSON encoder that handles custom types used here"""

    def default(self, obj: t.Any) -> t.Any:
        if isinstance(obj, np.ndarray):
            return obj.tolist()
        if isinstance(obj, Transformation):
            tf_dict = {
                "position": obj.translation,
                "orientation": obj.get_rotation(),
            }
            return tf_dict
        return json.JSONEncoder.default(self, obj)


def get_table_transform(transformer: ViconTransformer) -> Transformation:
    """Get pose of the table based on poses of the corner markers."""
    # table dimensions (assuming a standard table tennis table)
    # https://www.tabletennisspot.com/knowing-the-dimensions-of-table-tennis-table
    TABLE_LENGTH = 0.274
    TABLE_WIDTH = 1.525

    # IDs of the corners:
    #
    #  1┌─────────┐2
    #   │  y      │
    #   │  ▲      │
    #   │  │      │
    #   │  │    x │
    #   │  └───>  │
    #   │         │
    #  4└─────────┘3

    DX = TABLE_WIDTH / 2
    DY = TABLE_LENGTH / 2
    corner_vectors_table_frame = np.array(
        [
            [-DX, DY, 0],
            [DX, DY, 0],
            [DX, -DY, 0],
            [-DX, -DY, 0],
        ]
    )

    corner_poses_world = [
        transformer.get_transform(name) for name in TABLE_CORNER_SUBJECTS
    ]
    corner_positions_world = [c.translation for c in corner_poses_world]

    # For the table's position in world frame, simply use the centroid between the
    # corners.
    table_position_world = np.mean(corner_positions_world, axis=0)

    # Remove the translational part from the corner positions in world frame.  To my
    # understanding, this is not really necessary as it is anyway done as a first step
    # in the Kabsch algorithm, which is implemented in Rotation.align_vectors().
    # However, the documentation does not explicitly mention this and asks for vectors
    # rather then points, so better already do it here to be on the safe side (i.e. in
    # case they change the implementation in a future release).
    corner_vectors_world_frame = corner_positions_world - table_position_world

    # get rotation of the table in world frame
    table_rot, rssd = Rotation.align_vectors(
        corner_vectors_world_frame, corner_vectors_table_frame
    )

    logging.info("Table position: %s", table_position_world)
    logging.info("Table rotation (XYZ): %s", table_rot.as_euler("XYZ"))
    logging.info("Table rotation rssd: %s", rssd)

    table_tf = Transformation()
    table_tf.translation = table_position_world
    table_tf.set_rotation(table_rot.as_quat())

    return table_tf


def main() -> int:
    # parse arguments
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "host_or_file",
        type=str,
        help="Hostname of Vicon server or path to recorded file.",
    )
    parser.add_argument(
        "--origin-subject",
        "-o",
        type=str,
        default="rll_ping_base",
        help="Name of the origin marker (used as origin for the world frame).",
    )
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO)

    receiver: Receiver
    if (filename := pathlib.Path(args.host_or_file)).is_file():
        receiver = PlaybackReceiver(filename)
    else:
        config = ViconReceiverConfig()
        receiver = ViconReceiver(args.host_or_file, config)
        receiver.connect()

    transformer = ViconTransformer(receiver, args.origin_subject)
    transformer.wait_for_origin_subject_data()

    transformer.update()

    output = {
        "robot": transformer.get_transform(ROBOT_BASE_SUBJECT),
        "ball_machine": transformer.get_transform(BALL_MACHINE_SUBJECT),
        "table": get_table_transform(transformer),
    }

    print(json.dumps(output, indent=4, cls=JsonEncoder))

    return 0


if __name__ == "__main__":
    with contextlib.suppress(KeyboardInterrupt):
        sys.exit(main())
