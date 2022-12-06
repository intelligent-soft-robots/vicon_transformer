import json
import logging

import numpy as np
import zmq

# object names
# ['frame_number', 'frame_rate', 'latency', 'my_frame_number', 'num_subjects',
# 'on_time', 'subjectNames', 'subject_0', 'subject_1', 'subject_2', 'subject_3',
# 'subject_4', 'subject_5', 'subject_6', 'subject_7', 'subject_8', 'subject_9',
# 'time_stamp']


def T(R, p):
    tmp = np.eye(4)
    tmp[:3, :3] = np.array(R).reshape(3, 3)
    tmp[:3, 3:4] = np.array(p).reshape(3, 1)
    return tmp


def inv_T(T):
    invT = T.copy()
    invT[:3, :3] = T[:3, :3].T
    invT[:3, 3:4] = -invT[:3, :3].dot(T[:3, 3:4])
    return invT


class ViconJson:
    def __init__(
        self,
        fname="testViconFrameTableRot.json",  # 'testViconFrame.txt'
        ip="10.42.2.29",
        port="5555",
        timeout_in_ms=5000,
    ):
        self.log = logging.getLogger(__name__)

        self.zmq_connected = False
        self.sub = None
        self.context = None
        self.ip = ip
        self.port = port
        self.timeout_in_ms = timeout_in_ms
        self.T_origin_vicon = np.eye(4)
        # try connecting to zmq
        self.zmq_connect(self.ip, self.port, self.timeout_in_ms)
        # read frame from file when connection cannot be established
        if self.zmq_connected:
            self.json_obj = self.read_vicon_json_from_zmq()
            self.log.info("Vicon connected via zmq")
        else:
            self.json_obj = self.read_vicon_json_from_file(fname)
            self.log.info("Vicon initialised via test frame from file")
        self._init_origin()

    # init origin

    def _init_origin(self):
        originKey = "rll_ping_base"
        self.T_origin_vicon = inv_T(self.get_T(originKey))

    # connecting and reading frames

    def read_vicon_json_from_zmq(self):
        if not self.zmq_connected:
            self.log.error("read_vicon_json_from_zmq: connect before reading")
            return []
        else:  # read
            self.json_obj = self.sub.recv_json()
            return self.json_obj

    def zmq_connect(self, ip, port, timeout):
        self.log.debug("zmq_connect: connecting...")
        try:
            self.context = zmq.Context()
            self.sub = self.context.socket(zmq.SUB)
            self.sub.setsockopt(zmq.SUBSCRIBE, b"")
            self.sub.RCVTIMEO = self.timeout_in_ms  # wait 5s for new message

            address = f"tcp://{self.ip}:{self.port}"
            self.log.info("Connect to %s", address)
            self.sub.connect(address)

            if self.sub.closed is True:
                self.log.error("zmq_connect(): could not connect")
                return

            # test read frame until frame available or timeout
            n = 0
            while n < 10 or not self.zmq_connected:
                self.json_obj = self.sub.recv_json()
                if self.json_obj != []:
                    self.zmq_connected = True
                n = n + 1
            if self.zmq_connected:
                self.log.debug("zmq_connect(): connected")
            else:
                self.log.error("zmq_connect(): could not read a frame")
                self.zmq_disconnect()

        except Exception as e:
            self.log.error(f"zmq_connect(): could not connect: {e}")
            return

    def zmq_disconnect(self):
        self.log.debug("zmq_disconnect: disconnecting...")
        if self.zmq_connected:
            self.context.destroy()
            if self.sub.closed is True:
                self.zmq_connected = False
                self.log.info("zmq_disconnect: disconnected. Bye...")
            else:
                self.log.error("zmq_disconnect: disconnecting failed!")
        else:
            self.log.info("zmq_disconnect: already disconnected. Bye...")

    def read_vicon_json_from_file(self, fname):
        with open(fname) as json_file:
            r = json.load(json_file)
        return r

    # measure distances

    def print_distances(self):
        t_pos_1 = self.get_table1_T()[:3, -1]
        t_pos_2 = self.get_table2_T()[:3, -1]
        t_pos_3 = self.get_table3_T()[:3, -1]
        t_pos_4 = self.get_table4_T()[:3, -1]

        print("table lengths")
        print("1->4 ", np.linalg.norm(t_pos_1 - t_pos_4))
        print("2->3 ", np.linalg.norm(t_pos_2 - t_pos_3))
        print("1->2 ", np.linalg.norm(t_pos_1 - t_pos_2))
        print("4->3 ", np.linalg.norm(t_pos_4 - t_pos_3))

        print("distance origin -> robot vicon marker")
        t_rob_origin = self.get_robot_base_T()[:3, -1]
        print(t_rob_origin)

    # get object rotations and translations

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

    def get_robot_shoulder_T(self):
        # TODO: Orientation is left the same with the base orientation
        #       shoulder is located at [-.255,.0785,0] in base frame
        T_base_shoulder = np.eye(4)
        T_base_shoulder[:3, -1] = np.asarray(
            [-0.255, 0.0785, 0]
        )  # measured on real robot
        T_orig_base = self.get_robot_base_T()
        return T_orig_base @ T_base_shoulder

    def get_table_pos(self):
        t_pos_1 = self.get_table1_trans()
        t_pos_2 = self.get_table2_trans()
        t_pos_3 = self.get_table3_trans()
        t_pos_4 = self.get_table4_trans()
        t = (t_pos_1 + t_pos_2 + t_pos_3 + t_pos_4) / 4

        return t

    # Transformations
    def get_robot_base_T(self):
        return self.get_T("rll_muscle_base")

    def get_table1_T(self):
        return self.get_T("TT Platte_Eckteil 1")

    def get_table2_T(self):
        return self.get_T("TT Platte_Eckteil 2")

    def get_table3_T(self):
        return self.get_T("TT Platte_Eckteil 3")

    def get_table4_T(self):
        return self.get_T("TT Platte_Eckteil 4")

    # access json methods
    def get_T(self, key):
        # Returns homogenous transformation in origin frame
        idx = self.json_obj["subjectNames"].index(key)
        tr = 1e-3 * np.asarray(
            self.json_obj["subject_" + str(idx)]["global_translation"][0]
        ).reshape(3, 1)
        R = np.asarray(
            self.json_obj["subject_" + str(idx)]["global_rotation"]["matrix"][0]
        )
        return self.T_origin_vicon @ T(R, tr)
