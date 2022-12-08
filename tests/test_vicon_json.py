import pathlib
import numpy as np
import math

import pytest

from vicon_transformer import ViconJsonFile


@pytest.fixture
def test_data():
    return pathlib.PurePath(__file__).parent / "data"


def test_origin_init(test_data):
    def SO3_2_so3(R):
        theta = math.acos(np.clip((np.trace(R) - 1.0) / 2.0, -1.0, 1.0))
        if np.abs(theta) < 1e-6:
            return np.array([0.0, 0.0, 0.0], dtype="float").reshape(3, 1), theta

        w = 0.5 * np.array(
            [R[2, 1] - R[1, 2], R[0, 2] - R[2, 0], R[1, 0] - R[0, 1]], dtype="float"
        ).reshape(3, 1)
        return w, theta

    vT1 = ViconJsonFile(test_data / "test_frame1.json")
    vT2 = ViconJsonFile(test_data / "test_frame2.json")

    for key in vT1.json_obj["subjectNames"]:
        # the marker of the Ballmaschine is not very good, better ignore it here
        if key == "Marker Ballmaschine":
            continue

        T1 = vT1.get_T(key=key)
        T2 = vT2.get_T(key=key)
        delta_tr = T1[:3, -1] - T2[:3, -1]
        delta_R = T1[:3, :3].T @ T2[:3, :3]

        # translation error
        assert np.linalg.norm(delta_tr) < 0.0025, key
        # rotation error
        assert SO3_2_so3(delta_R)[1] < 0.02, key


def test_basic_transforms_ping_at_origin(test_data):
    # Load a file where the ping marker is at the origin.  This means the
    # transformations of all other markers should be exactly as in the file (since no
    # origin transformation is happening).
    # Note that the translation in the file is in millimetres, so this needs to be
    # converted to metres for the comparison.

    vicon = ViconJsonFile(test_data / "frame_ping_at_origin.json")

    np.testing.assert_array_almost_equal(
        vicon.get_robot_base_T(),
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
        vicon.get_table1_T(),
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
            [0, 0, 0, 1],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_table2_T(),
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
            [0, 0, 0, 1],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_table3_T(),
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
            [0, 0, 0, 1],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_table4_T(),
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
            [0, 0, 0, 1],
        ],
    )


def test_basic_transforms_ping_simple_translation(test_data):
    # Load a file where the ping marker has some translation from the origin but no
    # rotation.
    # Note that the translation in the file is in millimetres, so this needs to be
    # converted to metres for the comparison.

    vicon = ViconJsonFile(test_data / "frame_ping_simple_translation.json")

    np.testing.assert_array_almost_equal(
        vicon.get_robot_base_T(),
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
        vicon.get_table1_T(),
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
            [0, 0, 0, 1],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_table2_T(),
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
            [0, 0, 0, 1],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_table3_T(),
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
            [0, 0, 0, 1],
        ],
    )

    np.testing.assert_array_almost_equal(
        vicon.get_table4_T(),
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
            [0, 0, 0, 1],
        ],
    )


def test_get_table_pos(test_data):
    vicon = ViconJsonFile(test_data / "frame_ping_at_origin.json")

    np.testing.assert_array_almost_equal(
        vicon.get_table_pos(), [-253.98395753, 53.75158628, 31.22968497]
    )
