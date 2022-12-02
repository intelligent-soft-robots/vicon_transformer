from __future__ import print_function

from os.path import dirname, abspath, join
import sys
import numpy as np
import math

# Find code directory relative to our directory
THIS_DIR = dirname(__file__)
CODE_DIR = abspath(join(THIS_DIR, "..", "vicon_transformer"))
sys.path.append(CODE_DIR)

from vicon_transformer import ViconJson


def testOriginInit():
    def SO3_2_so3(R):
        theta = math.acos(np.clip((np.trace(R) - 1.0) / 2.0, -1.0, 1.0))
        if np.abs(theta) < 1e-6:
            return np.array([0.0, 0.0, 0.0], dtype="float").reshape(3, 1), theta

        w = 0.5 * np.array(
            [R[2, 1] - R[1, 2], R[0, 2] - R[2, 0], R[1, 0] - R[0, 1]], dtype="float"
        ).reshape(3, 1)
        return w, theta

    vT1 = ViconJson(fname="test_frame1.json", timeout_in_ms=0)
    vT2 = ViconJson(fname="test_frame2.json", timeout_in_ms=0)

    for key in vT1.json_obj["subjectNames"]:
        T1 = vT1.get_T(key=key)
        T2 = vT2.get_T(key=key)
        delta_tr = T1[:3, -1] - T2[:3, -1]
        delta_R = T1[:3, :3].T @ T2[:3, :3]
        print("Transl error norm: ", np.linalg.norm(delta_tr), "meter")
        print("Rot error norm: ", SO3_2_so3(delta_R)[1], "grad")
        print("T error\n", delta_R, "\n", delta_tr.T)
        print()


if __name__ == "__main__":
    testOriginInit()
