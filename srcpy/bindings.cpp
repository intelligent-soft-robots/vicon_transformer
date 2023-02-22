// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file Python bindings of the relevant C++ classes/functions.
 * @copyright 2022, Max Planck Gesellschaft.  All rights reserved.
 */
#include <sstream>

#include <pybind11/eigen.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <serialization_utils/cereal_json.hpp>

#include <vicon_transformer/errors.hpp>
#include <vicon_transformer/transform.hpp>
#include <vicon_transformer/types.hpp>
#include <vicon_transformer/vicon_receiver.hpp>
#include <vicon_transformer/vicon_transformer.hpp>

PYBIND11_MODULE(vicon_transformer_bindings, m)
{
    namespace py = pybind11;
    namespace vt = vicon_transformer;

    py::register_exception<vt::NotConnectedError>(
        m, "NotConnectedError", PyExc_RuntimeError);
    py::register_exception<vt::BadResultError>(
        m, "BadResultError", PyExc_RuntimeError);
    py::register_exception<vt::SubjectNotVisibleError>(
        m, "SubjectNotVisibleError", PyExc_RuntimeError);
    py::register_exception<vt::UnknownSubjectError>(
        m, "UnknownSubjectError", PyExc_RuntimeError);

    // TODO: add unit tests for bindings of Transformation
    py::class_<vt::Transformation>(m, "Transformation")
        .def(py::init<>(), py::call_guard<py::gil_scoped_release>())
        .def(py::init<Eigen::Quaterniond, Eigen::Vector3d>(),
             py::call_guard<py::gil_scoped_release>())
        .def(py::self * py::self)
        .def(py::self * Eigen::Vector3d())
        .def("apply",
             &vt::Transformation::apply,
             py::call_guard<py::gil_scoped_release>())
        .def("inverse",
             &vt::Transformation::inverse,
             py::call_guard<py::gil_scoped_release>())
        .def("matrix",
             &vt::Transformation::matrix,
             py::call_guard<py::gil_scoped_release>())
        .def_readwrite("translation", &vt::Transformation::translation)
        // there are no proper bindings for Eigen::Quaterniond, so as a simple
        // workaround provide a getter and setter that provide/expect the
        // quaternion as a list [x, y, z, w].
        .def("get_rotation",
             [](const vt::Transformation& self)
             {
                 std::array<double, 4> quat = {self.rotation.x(),
                                               self.rotation.y(),
                                               self.rotation.z(),
                                               self.rotation.w()};
                 return quat;
             })
        .def("set_rotation",
             [](vt::Transformation& self, const std::array<double, 4>& quat)
             {
                 self.rotation.x() = quat[0];
                 self.rotation.y() = quat[1];
                 self.rotation.z() = quat[2];
                 self.rotation.w() = quat[3];
             });

    py::class_<vt::SubjectData>(m, "SubjectData")
        .def(py::init<>())
        .def_readwrite("is_visible", &vt::SubjectData::is_visible)
        .def_readwrite("global_pose", &vt::SubjectData::global_pose)
        .def_readwrite("quality", &vt::SubjectData::quality);
    m.def("to_json", &serialization_utils::to_json<vt::SubjectData>);
    m.def("from_json", &serialization_utils::from_json<vt::SubjectData>);

    py::class_<vt::ViconFrame>(m, "ViconFrame")
        .def(py::init<>())
        .def_readwrite("frame_number", &vt::ViconFrame::frame_number)
        .def_readwrite("frame_rate", &vt::ViconFrame::frame_rate)
        .def_readwrite("latency", &vt::ViconFrame::latency)
        .def_readwrite("time_stamp", &vt::ViconFrame::time_stamp)
        .def_readwrite("subjects", &vt::ViconFrame::subjects)
        .def(
            "__str__",
            [](const vt::ViconFrame& vf)
            {
                std::stringstream stream;
                stream << vf;
                return stream.str();
            },
            py::call_guard<py::gil_scoped_release>());
    m.def("to_json", &serialization_utils::to_json<vt::ViconFrame>);
    m.def("from_json", &serialization_utils::from_json<vt::ViconFrame>);

    py::class_<vt::ViconReceiverConfig>(m, "ViconReceiverConfig")
        .def(py::init<>())
        .def_readwrite("enable_lightweight",
                       &vt::ViconReceiverConfig::enable_lightweight)
        .def_readwrite("buffer_size", &vt::ViconReceiverConfig::buffer_size)
        .def_readwrite("filtered_subjects",
                       &vt::ViconReceiverConfig::filtered_subjects);
    m.def("to_json", &serialization_utils::to_json<vt::ViconReceiverConfig>);
    m.def("from_json",
          &serialization_utils::from_json<vt::ViconReceiverConfig>);

    py::class_<vt::Receiver, std::shared_ptr<vt::Receiver>> PyReceiver(
        m, "Receiver");
    py::class_<vt::ViconReceiver,
               std::shared_ptr<vt::ViconReceiver>,
               vt::Receiver>(m, "ViconReceiver")
        .def(py::init<std::string, vt::ViconReceiverConfig>(),
             py::arg("host_name"),
             py::arg("config"),
             py::call_guard<py::gil_scoped_release>())
        .def("is_connected",
             &vt::ViconReceiver::is_connected,
             py::call_guard<py::gil_scoped_release>())
        .def("connect",
             &vt::ViconReceiver::connect,
             py::call_guard<py::gil_scoped_release>())
        .def("disconnect",
             &vt::ViconReceiver::disconnect,
             py::call_guard<py::gil_scoped_release>())
        .def("read",
             &vt::ViconReceiver::read,
             py::call_guard<py::gil_scoped_release>())
        .def("print_info",
             &vt::ViconReceiver::print_info,
             py::call_guard<py::gil_scoped_release>())
        .def("print_latency_info",
             &vt::ViconReceiver::print_latency_info,
             py::call_guard<py::gil_scoped_release>());
    py::class_<vt::JsonReceiver,
               std::shared_ptr<vt::JsonReceiver>,
               vt::Receiver>(m, "JsonReceiver")
        .def(py::init<const std::filesystem::path&>(),
             py::arg("filename"),
             py::call_guard<py::gil_scoped_release>())
        .def("read",
             &vt::JsonReceiver::read,
             py::call_guard<py::gil_scoped_release>());
    py::class_<vt::PlaybackReceiver,
               std::shared_ptr<vt::PlaybackReceiver>,
               vt::Receiver>(m, "PlaybackReceiver")
        .def(py::init<const std::filesystem::path&>(),
             py::arg("filename"),
             py::call_guard<py::gil_scoped_release>())
        .def("read",
             &vt::PlaybackReceiver::read,
             py::call_guard<py::gil_scoped_release>());

    py::class_<vt::ViconTransformer>(m, "ViconTransformer")
        .def(py::init<std::shared_ptr<vt::Receiver>, const std::string&>(),
             py::call_guard<py::gil_scoped_release>())
        .def("update",
             &vt::ViconTransformer::update,
             py::call_guard<py::gil_scoped_release>())
        .def("set_frame",
             &vt::ViconTransformer::set_frame,
             py::call_guard<py::gil_scoped_release>())
        .def("wait_for_origin_subject_data",
             &vt::ViconTransformer::wait_for_origin_subject_data,
             py::call_guard<py::gil_scoped_release>())
        .def("get_timestamp_ns",
             &vt::ViconTransformer::get_timestamp_ns,
             py::call_guard<py::gil_scoped_release>())
        .def("get_subject_names",
             &vt::ViconTransformer::get_subject_names,
             py::call_guard<py::gil_scoped_release>())
        .def("is_visible",
             &vt::ViconTransformer::is_visible,
             py::call_guard<py::gil_scoped_release>())
        .def("get_transform",
             &vt::ViconTransformer::get_transform,
             py::call_guard<py::gil_scoped_release>())
        .def("get_frame",
             &vt::ViconTransformer::get_frame,
             py::call_guard<py::gil_scoped_release>());
}
