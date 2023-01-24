"""Test o80 driver and standalone."""
import pathlib

import numpy as np
import pytest

import o80

from vicon_transformer.vicon_transformer_bindings import (
    JsonReceiver,
)
from vicon_transformer.pam_vicon_o80 import (
    FrontEnd,
    Subjects,
    start_standalone,
    stop_standalone,
)

SEGMENT_ID = "test_vicon"
ORIGIN_SUBJECT = "rll_ping_base"


@pytest.fixture
def test_data():
    return pathlib.PurePath(__file__).parent / "data"


@pytest.fixture
def burst_standalone(test_data):
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


def test_o80_basic(burst_standalone) -> None:
    # create a front end and receive one observation
    frontend = FrontEnd(SEGMENT_ID)
    obs = frontend.latest()
    vicon_frame = obs.get_extended_state()

    # check a few frames

    assert vicon_frame.subjects[Subjects.MUSCLE_BASE].is_visible
    np.testing.assert_array_almost_equal(
        vicon_frame.subjects[Subjects.MUSCLE_BASE].global_pose.matrix(),
        [
            [
                0.8663438846138151,
                0.4993031329659253,
                -0.012027260812682643,
                83.3450422755914 / 1000,
            ],
            [
                0.49936305903567846,
                -0.8663894341721914,
                0.002425618543688639,
                480.1439649956338 / 1000,
            ],
            [
                -0.009209172751897504,
                -0.008107389542971665,
                -0.9999247278530641,
                471.5935179506591 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    assert vicon_frame.subjects[Subjects.TABLE_CORNER_1].is_visible
    np.testing.assert_array_almost_equal(
        vicon_frame.subjects[Subjects.TABLE_CORNER_1].global_pose.matrix(),
        [
            [
                0.9447796559203805,
                -0.3274233104572087,
                0.01361534164865755,
                -2282.5873210084287 / 1000,
            ],
            [
                0.32746814862858703,
                0.94486152759689,
                -0.0011424977185985471,
                -1160.8202228673058 / 1000,
            ],
            [
                -0.012490532123690789,
                0.00553799932409894,
                0.9999066542286601,
                43.67459816726638 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    assert vicon_frame.subjects[Subjects.TABLE_CORNER_2].is_visible
    np.testing.assert_array_almost_equal(
        vicon_frame.subjects[Subjects.TABLE_CORNER_2].global_pose.matrix(),
        [
            [
                0.37591100914174547,
                0.926309291683833,
                -0.02533790335111176,
                -2806.0282763708333 / 1000,
            ],
            [
                -0.926655743238343,
                0.3757768243731537,
                -0.010045485922668693,
                273.557791462457 / 1000,
            ],
            [
                0.0002161699079075187,
                0.027255722412435353,
                0.9996284704160578,
                46.524908145107894 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    assert vicon_frame.subjects[Subjects.TABLE_CORNER_3].is_visible
    np.testing.assert_array_almost_equal(
        vicon_frame.subjects[Subjects.TABLE_CORNER_3].global_pose.matrix(),
        [
            [
                -0.9376639450735708,
                0.34581695781261873,
                0.0345970778866231,
                -226.65678645681226 / 1000,
            ],
            [
                -0.34348033219544655,
                -0.9372761978542612,
                0.05945242073091214,
                1218.4275599197388 / 1000,
            ],
            [
                0.052986672890201744,
                0.04386297556123178,
                0.9976314208518802,
                21.55445682535215 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    assert vicon_frame.subjects[Subjects.TABLE_CORNER_4].is_visible
    np.testing.assert_array_almost_equal(
        vicon_frame.subjects[Subjects.TABLE_CORNER_4].global_pose.matrix(),
        [
            [
                -0.372867823047438,
                -0.9277294428099088,
                0.01696076293922396,
                299.3365537085413 / 1000,
            ],
            [
                0.9277654203136207,
                -0.37305049237097526,
                -0.009200815836827561,
                -216.15878339196405 / 1000,
            ],
            [
                0.014863088715161506,
                0.012304921185810514,
                0.9998138214229968,
                25.164776745602744 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )
