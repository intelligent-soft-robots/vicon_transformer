# SPDX-License-Identifier: BSD-3-Clause
"""ROS Node that publishes TF transforms for Vicon objects."""
import rclpy
from geometry_msgs.msg import TransformStamped
from rclpy.node import Node
from tf2_ros import TransformBroadcaster

from vicon_transformer import (
    ViconReceiver,
    ViconReceiverConfig,
    ViconTransformer,
    SubjectNotVisibleError,
)


class FramePublisher(Node):
    def __init__(self) -> None:
        super().__init__("vicon_tf_publisher")

        self.vicon_host = (
            self.declare_parameter("vicon_host").get_parameter_value().string_value
        )
        self.origin_subject = (
            self.declare_parameter("origin_subject", "rll_ping_base")
            .get_parameter_value()
            .string_value
        )
        self.use_wall_time = (
            self.declare_parameter("use_wall_time", False)
            .get_parameter_value()
            .bool_value
        )

        # Initialize the transform broadcaster
        self.tf_broadcaster = TransformBroadcaster(self)

    def run(self):
        config = ViconReceiverConfig()

        with ViconReceiver(self.vicon_host, config) as receiver:
            vicon = ViconTransformer(receiver, self.origin_subject)
            vicon.wait_for_origin_subject_data()

            while rclpy.ok():
                vicon.update()

                for subject_name in vicon.get_subject_names():
                    try:
                        transform = vicon.get_transform(subject_name)
                    except SubjectNotVisibleError:
                        # skip the subject if it is not present
                        continue

                    tf_msg = TransformStamped()

                    if self.use_wall_time:
                        tf_msg.header.stamp = self.get_clock().now().to_msg()
                    else:
                        timestamp_s = vicon.get_timestamp_ns() / 1e9
                        tf_msg.header.stamp.sec = int(timestamp_s)
                        tf_msg.header.stamp.nanosec = int((timestamp_s % 1) * 1e9)

                    tf_msg.header.frame_id = "world"
                    tf_msg.child_frame_id = subject_name

                    (
                        tf_msg.transform.translation.x,
                        tf_msg.transform.translation.y,
                        tf_msg.transform.translation.z,
                    ) = transform.translation

                    (
                        tf_msg.transform.rotation.x,
                        tf_msg.transform.rotation.y,
                        tf_msg.transform.rotation.z,
                        tf_msg.transform.rotation.w,
                    ) = transform.get_rotation()

                    # Send the transformation
                    self.tf_broadcaster.sendTransform(tf_msg)


def main():
    rclpy.init()
    node = FramePublisher()
    try:
        node.run()
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == "__main__":
    main()
