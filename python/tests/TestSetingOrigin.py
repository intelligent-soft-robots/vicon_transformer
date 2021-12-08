from __future__ import print_function

from os.path import dirname, abspath, join
import sys
import numpy as np

# Find code directory relative to our directory
THIS_DIR = dirname(__file__)
CODE_DIR = abspath(join(THIS_DIR, '..', 'vicon_transformer'))
sys.path.append(CODE_DIR)

from vicon_transformer import ViconJson


def testOriginInit():
    vT1 = ViconJson(fname='test_frame1.json', timeout_in_ms=0)
    vT2 = ViconJson(fname='test_frame2.json', timeout_in_ms=0)

    for key in  vT1.json_obj['subjectNames']:
        diff = vT1.get_transl(key=key).T-vT2.get_transl(key=key).T
        diff2 = vT1.get_rot_mat(key=key).T-vT2.get_rot_mat(key=key).T
        print('position error', np.linalg.norm(diff))
        print('rotation error\n', diff2)
        print()


if __name__ == "__main__":
    testOriginInit()