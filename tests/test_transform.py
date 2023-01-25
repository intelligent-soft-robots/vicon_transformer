# SPDX-License-Identifier: BSD-3-Clause
"""Test the Transformation class."""
import numpy as np

from vicon_transformer.transform import Transformation, Rotation


def test_identity():
    tf_id = Transformation.identity()
    np.testing.assert_array_almost_equal(tf_id.apply([1.0, 2.0, 3.0]), [1.0, 2.0, 3.0])

    tf_id = Transformation()
    np.testing.assert_array_almost_equal(tf_id.apply([1.0, 2.0, 3.0]), [1.0, 2.0, 3.0])


def test_rotation():
    tf = Transformation(Rotation.from_euler("z", 90, degrees=True))
    np.testing.assert_array_almost_equal(tf.apply([1, 0, 0]), [0, 1, 0])


def test_rotation_input_formats():
    # it should be possible to give the rotation either as Rotation instance or as
    # simple quaternion
    quaternion = (0.37642246, -0.34991817, 0.51694777, -0.68456439)
    tf1 = Transformation(Rotation.from_quat(quaternion))
    tf2 = Transformation(quaternion)

    np.testing.assert_array_almost_equal(tf1.rotation.as_quat(), quaternion)
    np.testing.assert_array_almost_equal(tf2.rotation.as_quat(), quaternion)


def test_translation():
    tf = Transformation(translation=[1, 2, 3])
    np.testing.assert_array_almost_equal(tf.apply([1, 0, -10]), [2, 2, -7])


def test_full_tf():
    tf = Transformation(Rotation.from_euler("z", 90, degrees=True), [0, 0, 2])
    np.testing.assert_array_almost_equal(tf.apply([1, 0, 0]), [0, 1, 2])


def test_as_matrix():
    tf = Transformation(Rotation.from_euler("z", 90, degrees=True), [0.1, 0.2, 0.3])
    np.testing.assert_array_almost_equal(
        tf.as_matrix(), [[0, -1, 0, 0.1], [1, 0, 0, 0.2], [0, 0, 1, 0.3], [0, 0, 0, 1]]
    )


def test_compose():
    tf1 = Transformation(Rotation.from_euler("z", 90, degrees=True), [0.1, 0.2, 0.3])
    tf2 = Transformation(
        Rotation.from_euler("xyz", (12, 34, 2), degrees=True), [2.1, -0.2, 0.0]
    )

    mat_comp = tf1.as_matrix() @ tf2.as_matrix()
    tf_comp = tf1 * tf2
    np.testing.assert_array_almost_equal(mat_comp, tf_comp.as_matrix())


def test_inv():
    tf = Transformation(
        Rotation.from_euler("xyz", (12, 34, 2), degrees=True), [2.1, -0.2, 0.0]
    )
    inv_tf = tf.inv()

    inv_mat = np.linalg.inv(tf.as_matrix())
    np.testing.assert_array_almost_equal(inv_tf.as_matrix(), inv_mat)
