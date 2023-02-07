# SPDX-License-Identifier: BSD-3-Clause
"""Simple node for testing tf."""
import contextlib
import typing

import numpy as np

import rclpy
from rclpy.node import Node
from tf2_ros import TransformException
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener

from vicon_transformer.transform import Transformation, Rotation

if typing.TYPE_CHECKING:
    from geometry_msgs.msg import TransformStamped


def msg_to_transform(msg: TransformStamped) -> Transformation:
    """Convert a transform message to a Transformation instance."""
    rot = Rotation.from_quat(
        [
            msg.transform.rotation.x,
            msg.transform.rotation.y,
            msg.transform.rotation.z,
            msg.transform.rotation.w,
        ]
    )
    trans = [
        msg.transform.translation.x,
        msg.transform.translation.y,
        msg.transform.translation.z,
    ]

    return Transformation(rot, trans)


class FrameListener(Node):
    def __init__(self):
        super().__init__("vicon_tf2_frame_listener")

        # Declare and acquire `target_frame` parameter
        self.target_frame = (
            self.declare_parameter("target_frame", "rll_muscle_base")
            .get_parameter_value()
            .string_value
        )

        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        # Call on_timer function every second
        self.timer = self.create_timer(1.0, self.on_timer)

    def on_timer(self):
        # Store frame names in variables that will be used to
        # compute transformations
        from_frame_rel = self.target_frame
        to_frame_rel = "world"

        # Look up for the transformation between frames
        try:
            transform_msg = self.tf_buffer.lookup_transform(
                to_frame_rel, from_frame_rel, rclpy.time.Time()
            )
            transform = msg_to_transform(transform_msg)
        except TransformException as ex:
            self.get_logger().exception(
                f"Could not transform {to_frame_rel} to {from_frame_rel}: {ex}"
            )
            return

        point = np.array([0.0, 0.0, 1.0])
        point_transformed = transform.apply(point)

        print()
        print(f"base ({self.target_frame}): {transform.translation}")
        print(f"point: {point_transformed}")


def main():
    rclpy.init()
    node = FrameListener()
    with contextlib.suppress(KeyboardInterrupt):
        rclpy.spin(node)

    rclpy.shutdown()


if __name__ == "__main__":
    main()
