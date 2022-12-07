import rclpy
from geometry_msgs.msg import TransformStamped
from rclpy.node import Node
from tf2_ros import TransformBroadcaster

from vicon_transformer import ViconJsonZmq
from vicon_transformer.errors import SubjectNotPresentError


class FramePublisher(Node):
    def __init__(self):
        super().__init__("vicon_tf_publisher")

        self.zmq_address = (
            self.declare_parameter("zmq_address", "tcp://localhost:5555")
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
        vicon = ViconJsonZmq(self.zmq_address)

        while rclpy.ok():
            vicon.read()

            for subject_name in vicon.get_subject_names():
                try:
                    transform = vicon.get_T(subject_name)
                except SubjectNotPresentError:
                    # skip the subject if it is not present
                    continue

                tf_msg = TransformStamped()

                if self.use_wall_time:
                    tf_msg.header.stamp = self.get_clock().now().to_msg()
                else:
                    timestamp = vicon.get_timestamp()
                    tf_msg.header.stamp.sec = int(timestamp)
                    tf_msg.header.stamp.nanosec = int((timestamp % 1) * 1e9)

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
                ) = transform.rotation.as_quat()

                # Send the transformation
                self.tf_broadcaster.sendTransform(tf_msg)


def main():
    rclpy.init()
    node = FramePublisher()
    try:
        # rclpy.spin(node)
        node.run()
    except KeyboardInterrupt:
        pass

    rclpy.shutdown()


if __name__ == "__main__":
    main()
