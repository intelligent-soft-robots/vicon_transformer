# SPDX-License-Identifier: BSD-3-Clause
"""Wrapper around PAM Vicon o80 front end."""
import logging
import typing as t

import numpy as np
from scipy.spatial.transform import Rotation

from vicon_transformer import pam_vicon_o80, SubjectNotVisibleError, SubjectData
from spatial_transformation import Transformation

from vicon_transformer.pam_vicon_o80 import get_subject_names, Subjects


class NoFrameDataError(RuntimeError):
    """Error indicating that called method needs Vicon frame data."""

    def __init__(self) -> None:
        super().__init__("No frame data.  Call `update()` first.")


def get_table_pose(
    corner_positions_world: t.Sequence[np.ndarray],
    *,
    yaw_only: bool = False,
    table_length: float = 2.740,
    table_width: float = 1.525,
) -> Transformation:
    """Get pose of the table based on positions of the corner markers.

    Expected order of the table corners:

    ::

        1┌───────────┐2
         │           │
         │   y       │     l
         │   ▲       │     e
         │   │       │     n
         │   │    x  │     g
         │   └───>   │     t
         │           │     h
         │           │
        4└───────────┘3

             width


    The default values for table length and width are based on the ITTF rules [1].


    [1] https://documents.ittf.sport/document/284


    Args:
        corner_positions_world:  Positions of the table corners in world frame.  See
            above for the expected order.
        yaw_only:  If true, only the yaw angle of the tables orientation is used
            (i.e. assume that the table is flat on the ground).
        table_length:  Length of the table in metres.
        table_width:  Width of the table in metres.
    """
    dx = table_width / 2
    dy = table_length / 2
    # see docstring for expected order of corners
    corner_vectors_table_frame = np.array(
        [
            [-dx, dy, 0],
            [dx, dy, 0],
            [dx, -dy, 0],
            [-dx, -dy, 0],
        ]
    )

    # For the table's position in world frame, simply use the centroid between the
    # corners.
    table_position_world = np.mean(corner_positions_world, axis=0)

    # Remove the translational part from the corner positions in world frame.  To my
    # understanding, this is not really necessary as it is anyway done as a first
    # step in the Kabsch algorithm, which is implemented in
    # Rotation.align_vectors(). However, the documentation does not explicitly
    # mention this and asks for vectors rather then points, so better already do it
    # here to be on the safe side (i.e. in case they change the implementation in a
    # future release).
    corner_vectors_world_frame = corner_positions_world - table_position_world

    # get rotation of the table in world frame
    table_rot, rssd = Rotation.align_vectors(
        corner_vectors_world_frame, corner_vectors_table_frame
    )

    logging.info("Table position: %s", table_position_world)
    logging.info("Table rotation (XYZ): %s", table_rot.as_euler("XYZ"))
    logging.info("Table rotation rssd: %s", rssd)

    if yaw_only:
        logging.info("Only use yaw angle of table rotation.")
        table_rot_euler = table_rot.as_euler("XYZ")
        table_rot = Rotation.from_euler("Z", table_rot_euler[2])

    return Transformation(table_rot, table_position_world)


class PamVicon:
    """Wrapper around o80 FrontEnd to more easily access PAM Vicon data."""

    ROBOT_BASE_SUBJECT = pam_vicon_o80.Subjects.MUSCLE_BASE
    TABLE_CORNER_SUBJECTS = (
        pam_vicon_o80.Subjects.TABLE_CORNER_1,
        pam_vicon_o80.Subjects.TABLE_CORNER_2,
        pam_vicon_o80.Subjects.TABLE_CORNER_3,
        pam_vicon_o80.Subjects.TABLE_CORNER_4,
    )

    def __init__(self, segment_id: str) -> None:
        """
        Args:
            segment_id: Shared memory segment ID used by the o80 back end.
        """
        self.frontend = pam_vicon_o80.FrontEnd(segment_id)
        self._frame: t.Optional[pam_vicon_o80.FixedSizeViconFrame] = None

    def update(self) -> None:
        """Update with latest Vicon data."""
        self._frame = self.frontend.latest().get_extended_state()

    def _get_subject(self, index: Subjects) -> SubjectData:
        if self._frame is None:
            raise NoFrameDataError

        subject_data = self._frame.subjects[index]

        if not subject_data.is_visible:
            raise SubjectNotVisibleError(get_subject_names()[index])

        return subject_data

    def get_table_pose(self, yaw_only: bool = False) -> Transformation:
        """Get pose of the table based on poses of the corner markers.

        Args:
            yaw_only:  If true, only the yaw angle of the tables orientation is used
                (i.e. assume that the table is flat on the ground).
        """
        corner_poses_world = [
            self._get_subject(sub).global_pose for sub in self.TABLE_CORNER_SUBJECTS
        ]
        corner_positions_world = [c.translation for c in corner_poses_world]

        return get_table_pose(corner_positions_world, yaw_only=yaw_only)

    def get_robot_pose(self) -> Transformation:
        """Get pose of the robot base."""
        pose = self._get_subject(self.ROBOT_BASE_SUBJECT).global_pose
        return Transformation.from_cpp(pose)
