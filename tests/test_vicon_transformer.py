# SPDX-License-Identifier: BSD-3-Clause
"""Test Python bindings of ViconTransformer"""
import pathlib

import numpy as np
import pytest

from vicon_transformer import ViconTransformer, SubjectNotVisibleError
from vicon_transformer.vicon_transformer_bindings import JsonReceiver


ORIGIN_SUBJECT = "rll_ping_base"


@pytest.fixture()
def test_data():
    return pathlib.PurePath(__file__).parent / "data"


def test_timestamp(test_data) -> None:
    vicon = ViconTransformer(
        JsonReceiver(test_data / "test_frame1.json"), ORIGIN_SUBJECT
    )
    vicon.update()

    assert vicon.get_timestamp_ns() == 1638538681615901200


def test_origin_init(test_data) -> None:
    vt1 = ViconTransformer(JsonReceiver(test_data / "test_frame1.json"), ORIGIN_SUBJECT)
    vt1.update()
    vt2 = ViconTransformer(JsonReceiver(test_data / "test_frame2.json"), ORIGIN_SUBJECT)
    vt2.update()

    for name in vt1.get_subject_names():
        # the marker of the Ballmaschine is not very good, better ignore it here
        if name == "Marker Ballmaschine":
            continue

        t1 = vt1.get_transform(name)
        t2 = vt2.get_transform(name)
        delta_tr = t1.translation - t2.translation
        delta_rot = t1.rotation.inv() * t2.rotation

        # translation error
        assert np.linalg.norm(delta_tr) < 0.0025, name
        # rotation error
        assert delta_rot.magnitude() < 0.02, name


def test_basic_transforms_ping_at_origin(test_data) -> None:
    # Load a file where the ping marker is at the origin.  This means the
    # transformations of all other markers should be exactly as in the file (since no
    # origin transformation is happening).
    # Note that the translation in the file is in millimetres, so this needs to be
    # converted to metres for the comparison.

    vicon = ViconTransformer(
        JsonReceiver(test_data / "frame_ping_at_origin.json"), ORIGIN_SUBJECT
    )
    vicon.update()

    np.testing.assert_array_almost_equal(
        vicon.get_transform("rll_muscle_base").as_matrix(),
        [
            [
                0.8663438846138151,
                0.4993031329659253,
                -0.012027260812682643,
                1083.3450422755914 / 1000,
            ],
            [
                0.49936305903567846,
                -0.8663894341721914,
                0.002425618543688639,
                505.1439649956338 / 1000,
            ],
            [
                -0.009209172751897504,
                -0.008107389542971665,
                -0.9999247278530641,
                468.5935179506591 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 1").as_matrix(),
        [
            [
                0.9447796559203805,
                -0.3274233104572087,
                0.01361534164865755,
                -1282.5873210084287 / 1000,
            ],
            [
                0.32746814862858703,
                0.94486152759689,
                -0.0011424977185985471,
                -1135.8202228673058 / 1000,
            ],
            [
                -0.012490532123690789,
                0.00553799932409894,
                0.9999066542286601,
                40.67459816726638 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 2").as_matrix(),
        [
            [
                0.37591100914174547,
                0.926309291683833,
                -0.02533790335111176,
                -1806.0282763708333 / 1000,
            ],
            [
                -0.926655743238343,
                0.3757768243731537,
                -0.010045485922668693,
                298.557791462457 / 1000,
            ],
            [
                0.0002161699079075187,
                0.027255722412435353,
                0.9996284704160578,
                43.524908145107894 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 3").as_matrix(),
        [
            [
                -0.9376639450735708,
                0.34581695781261873,
                0.0345970778866231,
                773.3432135431877 / 1000,
            ],
            [
                -0.34348033219544655,
                -0.9372761978542612,
                0.05945242073091214,
                1243.4275599197388 / 1000,
            ],
            [
                0.052986672890201744,
                0.04386297556123178,
                0.9976314208518802,
                18.55445682535215 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 4").as_matrix(),
        [
            [
                -0.372867823047438,
                -0.9277294428099088,
                0.01696076293922396,
                1299.3365537085413 / 1000,
            ],
            [
                0.9277654203136207,
                -0.37305049237097526,
                -0.009200815836827561,
                -191.15878339196405 / 1000,
            ],
            [
                0.014863088715161506,
                0.012304921185810514,
                0.9998138214229968,
                22.164776745602744 / 1000,
            ],
            [0.0, 0.0, 0.0, 1.0],
        ],
    )


def test_basic_transforms_ping_simple_translation(test_data) -> None:
    # Load a file where the ping marker has some translation from the origin but no
    # rotation.
    # Note that the translation in the file is in millimetres, so this needs to be
    # converted to metres for the comparison.

    vicon = ViconTransformer(
        JsonReceiver(test_data / "frame_ping_simple_translation.json"), ORIGIN_SUBJECT
    )
    vicon.update()

    np.testing.assert_array_almost_equal(
        vicon.get_transform("rll_muscle_base").as_matrix(),
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

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 1").as_matrix(),
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

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 2").as_matrix(),
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

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 3").as_matrix(),
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

    np.testing.assert_array_almost_equal(
        vicon.get_transform("TT Platte_Eckteil 4").as_matrix(),
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


def test_subject_not_present(test_data) -> None:
    vicon = ViconTransformer(
        JsonReceiver(test_data / "frame_with_missing_subjects.json"), ORIGIN_SUBJECT
    )
    vicon.update()

    with pytest.raises(SubjectNotVisibleError, match="rll_muscle_racket"):
        vicon.get_transform("rll_muscle_racket")
