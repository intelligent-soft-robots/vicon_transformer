import pathlib
import numpy as np
import math

import pytest

from vicon_transformer import ViconJson  # noqa


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

    vT1 = ViconJson(fname=test_data / "test_frame1.json", timeout_in_ms=0)
    vT2 = ViconJson(fname=test_data / "test_frame2.json", timeout_in_ms=0)

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
