import json
import logging
import typing

import numpy as np

from .receiver import ZmqJsonReceiver
from .transform import Transformation, Rotation


class ViconJsonBase:
    FORMAT_VERSION = 2

    def __init__(self) -> None:
        self.log = logging.getLogger(__name__)
        self.T_origin_vicon = Transformation.identity()

        self.json_obj: typing.Dict

    def _init_origin(self) -> None:
        originKey = "rll_ping_base"
        self.T_origin_vicon = self.get_T(originKey).inv()

    def _check_format_version(self, record):
        format_version = record.get("format_version")
        if format_version != self.FORMAT_VERSION:
            raise RuntimeError(
                f"Incompatible format version '{format_version}'."
                f"  Expected {self.FORMAT_VERSION}."
            )

    def read(self):
        raise NotImplementedError()

    def get_subject_names(self) -> typing.List[str]:
        return list(self.json_obj["subjects"].keys())

    def print_distances(self) -> None:
        t_pos_1 = self.get_table1_T().translation
        t_pos_2 = self.get_table2_T().translation
        t_pos_3 = self.get_table3_T().translation
        t_pos_4 = self.get_table4_T().translation

        print("table lengths")
        print("1->4 ", np.linalg.norm(t_pos_1 - t_pos_4))
        print("2->3 ", np.linalg.norm(t_pos_2 - t_pos_3))
        print("1->2 ", np.linalg.norm(t_pos_1 - t_pos_2))
        print("4->3 ", np.linalg.norm(t_pos_4 - t_pos_3))

        print("distance origin -> robot vicon marker")
        t_rob_origin = self.get_robot_base_T().translation
        print(t_rob_origin)

    def get_timestamp(self) -> float:
        return self.json_obj["time_stamp"] / 1e9

    def get_robot_rot(self):
        # TODO
        r_rot_mat = self.get_robot_base_T()[:3, :3]
        # rotate robot base vicon frame to robot shoulder frame
        # 1) z axis 180 deg by flipping x & y
        r_tmp = r_rot_mat[1, :].copy()
        r_rot_mat[1, :] = r_rot_mat[0, :].copy()
        r_rot_mat[0, :] = r_tmp.copy()
        # rotate by 90 deg around z
        rot_90z = np.asarray([[0, -1, 0], [1, 0, 0], [0, 0, 1]])
        r_rot_mat = r_rot_mat @ rot_90z
        # bring into xy_axes form
        r_rot_xy_ = np.concatenate((r_rot_mat[0, :].T, r_rot_mat[1, :].T), axis=0)

        return np.squeeze(np.asarray(r_rot_xy_))

    def get_robot_shoulder_T(self) -> Transformation:
        # TODO: Orientation is left the same with the base orientation
        #       shoulder is located at [-.255,.0785,0] in base frame (measured on real
        #       robot)
        T_base_shoulder = Transformation(translation=[-0.255, 0.0785, 0])
        T_orig_base = self.get_robot_base_T()
        return T_orig_base * T_base_shoulder

    def get_table_pos(self) -> np.ndarray:
        t_pos_1 = self.get_table1_T().translation
        t_pos_2 = self.get_table2_T().translation
        t_pos_3 = self.get_table3_T().translation
        t_pos_4 = self.get_table4_T().translation
        t = (t_pos_1 + t_pos_2 + t_pos_3 + t_pos_4) / 4

        return t

    # Transformations
    def get_robot_base_T(self) -> Transformation:
        return self.get_T("rll_muscle_base")

    def get_table1_T(self) -> Transformation:
        return self.get_T("TT Platte_Eckteil 1")

    def get_table2_T(self) -> Transformation:
        return self.get_T("TT Platte_Eckteil 2")

    def get_table3_T(self) -> Transformation:
        return self.get_T("TT Platte_Eckteil 3")

    def get_table4_T(self) -> Transformation:
        return self.get_T("TT Platte_Eckteil 4")

    # access json methods
    def get_T(self, subject_name: str) -> Transformation:
        # Returns homogenous transformation in origin frame

        subject_data = self.json_obj["subjects"][subject_name]
        translation = 1e-3 * np.asarray(subject_data["global_translation"][0])
        rotation = Rotation(subject_data["global_rotation"]["quaternion"][0])
        tf = Transformation(rotation, translation)

        return self.T_origin_vicon * tf


class ViconJsonZmq(ViconJsonBase):
    def __init__(
        self,
        address="tcp://10.42.2.29:5555",  # TODO: do not use this as default here
        timeout_in_ms=5000,
    ):
        super().__init__()

        self.receiver = ZmqJsonReceiver(address, timeout_in_ms)
        self.receiver.connect()
        self.json_obj = self.read()
        self.log.info("Vicon connected via zmq")

        self._init_origin()

    def read(self):
        record = self.receiver.read()
        self._check_format_version(record)
        self.json_obj = record
        return self.json_obj


class ViconJsonFile(ViconJsonBase):
    def __init__(
        self,
        filename,
    ):
        super().__init__()

        self.json_obj = self.read_vicon_json_from_file(filename)
        self.log.info("Vicon initialised via test frame from file %s", filename)

        self._init_origin()

    def read(self):
        return self.json_obj

    def read_vicon_json_from_file(self, fname):
        with open(fname) as json_file:
            r = json.load(json_file)

        self._check_format_version(r)

        return r
