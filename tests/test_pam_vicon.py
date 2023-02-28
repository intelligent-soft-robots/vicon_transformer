# SPDX-License-Identifier: BSD-3-Clause
"""Test functions/classes of the pam_vicon module."""
import pathlib

import pytest
from numpy.testing import assert_array_almost_equal
from scipy.spatial.transform import Rotation

import o80
from vicon_transformer.vicon_transformer_bindings import (
    JsonReceiver,
)
from vicon_transformer.pam_vicon_o80 import (
    start_standalone,
    stop_standalone,
)

from pam_vicon_o80 import pam_vicon


TABLE_LENGTH = 2.740
TABLE_WIDTH = 1.525
SEGMENT_ID = "test_vicon"
ORIGIN_SUBJECT = "rll_ping_base"


@pytest.fixture()
def test_data():
    return pathlib.PurePath(__file__).parent / "data"


@pytest.fixture()
def _standalone(test_data):
    # Set up a standalone with a JsonReceiver, that always provides the observation
    # loaded from the file
    receiver = JsonReceiver(test_data / "frame_ping_simple_translation.json")

    o80.clear_shared_memory(SEGMENT_ID)
    frequency = 10
    burst = False
    start_standalone(SEGMENT_ID, frequency, burst, receiver, ORIGIN_SUBJECT)

    yield

    # tear down
    stop_standalone(SEGMENT_ID)


def test_get_table_pose_at_origin():
    DX = TABLE_WIDTH / 2
    DY = TABLE_LENGTH / 2
    corners = [
        [-DX, DY, 0],
        [DX, DY, 0],
        [DX, -DY, 0],
        [-DX, -DY, 0],
    ]

    pose = pam_vicon.get_table_pose(corners)

    assert_array_almost_equal(pose.translation, [0, 0, 0])
    assert_array_almost_equal(pose.rotation.as_quat(), [0, 0, 0, 1])


def test_get_table_pose_translated():
    DX = TABLE_WIDTH / 2
    DY = TABLE_LENGTH / 2
    corners = [
        [-DX + 1.2, DY - 0.7, 1.4],
        [DX + 1.2, DY - 0.7, 1.4],
        [DX + 1.2, -DY - 0.7, 1.4],
        [-DX + 1.2, -DY - 0.7, 1.4],
    ]

    pose = pam_vicon.get_table_pose(corners)

    assert_array_almost_equal(pose.translation, [1.2, -0.7, 1.4])
    assert_array_almost_equal(pose.rotation.as_quat(), [0, 0, 0, 1])


def test_get_table_pose_rotated():
    DX = TABLE_WIDTH / 2
    DY = TABLE_LENGTH / 2
    # rotated by +90°
    corners = [
        [-DY, -DX, 0],
        [-DY, DX, 0],
        [DY, DX, 0],
        [DY, -DX, 0],
    ]

    pose = pam_vicon.get_table_pose(corners)

    assert_array_almost_equal(pose.translation, [0, 0, 0])
    assert_array_almost_equal(pose.rotation.as_euler("XYZ", degrees=True), [0, 0, 90])


def test_get_table_pose_transformed():
    DX = TABLE_WIDTH / 2
    DY = TABLE_LENGTH / 2
    # rotated by +90°
    corners = [
        [-DY + 1.2, -DX - 0.7, 1.4],
        [-DY + 1.2, DX - 0.7, 1.4],
        [DY + 1.2, DX - 0.7, 1.4],
        [DY + 1.2, -DX - 0.7, 1.4],
    ]

    pose = pam_vicon.get_table_pose(corners)

    assert_array_almost_equal(pose.translation, [1.2, -0.7, 1.4])
    assert_array_almost_equal(pose.rotation.as_euler("XYZ", degrees=True), [0, 0, 90])


def test_get_table_pose_yaw_only():
    DX = TABLE_WIDTH / 2
    DY = TABLE_LENGTH / 2
    # rotated by +90°
    corners = [
        [-DY, -DX, 0],
        [-DY, DX, 0],
        [DY, DX, 0],
        [DY, -DX, 0],
    ]

    # add rotational perturbation that does not affect Z-axis
    rot = Rotation.from_euler("XY", (1.2, -0.8), degrees=True)
    corners = rot.apply(corners)

    pose_full = pam_vicon.get_table_pose(corners)
    pose_yaw_only = pam_vicon.get_table_pose(corners, yaw_only=True)

    assert_array_almost_equal(
        pose_full.rotation.as_euler("XYZ", degrees=True), (1.2, -0.8, 90)
    )
    assert_array_almost_equal(
        pose_yaw_only.rotation.as_euler("XYZ", degrees=True), (0, 0, 90)
    )


@pytest.mark.usefixtures("_standalone")
def test_pam_vicon_poses():
    pv = pam_vicon.PamVicon(SEGMENT_ID)

    # without calling pv.update() first
    with pytest.raises(pam_vicon.NoFrameDataError):
        pv.get_robot_pose()

    pv.update()

    robot = pv.get_robot_pose()
    table = pv.get_table_pose(yaw_only=False)
    table_yaw_only = pv.get_table_pose(yaw_only=True)

    assert_array_almost_equal(
        robot.translation,
        (
            1.0833450422755915 - 1.0,
            0.5051439649956337 - 0.025,
            0.46859351795065907 + 0.003,
        ),
    )
    assert_array_almost_equal(
        robot.rotation.as_quat(),
        (
            0.9660044056109515,
            0.25845280471831683,
            -0.0054959463541859105,
            -0.002725921337801426,
        ),
    )

    assert_array_almost_equal(table.translation, [-1.25398396, 0.02875159, 0.03422968])
    assert_array_almost_equal(
        table.rotation.as_euler("XYZ", degrees=True),
        [-0.16909139, 0.42076322, 110.1044683],
    )

    assert_array_almost_equal(
        table_yaw_only.translation, [-1.25398396, 0.02875159, 0.03422968]
    )
    assert_array_almost_equal(
        table_yaw_only.rotation.as_euler("XYZ", degrees=True),
        [0, 0, 110.1044683],
    )
